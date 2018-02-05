#ifndef ThreadPool_hpp
#define ThreadPool_hpp
#include <vector>               
#include <queue>                // queue
#include <memory>               // make_shared
#include <stdexcept>            // runtime_error
#include <thread>               // thread
#include <mutex>                // mutex,        unique_lock
#include <condition_variable>   // condition_variable
#include <future>               // future,      packaged_task
#include <functional>           // function,     bind
#include <utility>              // move,         forward

using namespace std;

class ThreadPool {
public:
	inline ThreadPool(size_t threads) : stop(false)   //在线程池中创建threads个工作线程
	{
		for(size_t i=0;i<threads;++i)   //启动工作线程
			workers.emplace_back([this]   //此处的lambda表达式捕获this，即线程池实例
		{
			for (;;)  //循环避免虚假唤醒
			{
				function<void()> task; {   // 定义函数对象的容器, 存储任意的返回类型为 void 参数表为空的函数
					unique_lock<mutex> lock(this->queue_mutex);   //创建互斥锁
					this->condition.wait(lock,   //阻塞当前线程，知道condition_variable被唤醒
						[this] {return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())     // 如果当前线程池已经结束且等待任务队列为空, 则应该直接返回
						return;
					task = move(this->tasks.front());   // 否则就让任务队列的队首任务作为需要执行的任务出队
					this->tasks.pop();
				}
				task();   //执行当前任务
			}
		});
	}
	template<typename F,typename... Args >  //向线程池中增加线程
	auto enqueue(F&& f, Args&&... args)    //F&&进行右值引用
		->future<typename result_of<F(Args...)>::type> {
			using return_type = typename  result_of<F(Args...)>::type;   //指导任务返回类型
			auto task = make_shared<packaged_task<return_type()>>(   //获得当前任务
				bind(forward<F>(f),forward<Args>(args)...)
				);
			future<return_type> res = task->get_future();   // 获得 std::future 对象以供实施线程同步
			{
				unique_lock<mutex> lock(queue_mutex);
				if (stop)    //禁止在线程池停止后加入新的线程
				{
					throw runtime_error("enqueue on stopped ThreadPool");
				}
				tasks.emplace([task] {(*task)(); });   //将线程添加到执行任务队列中
			}
			condition.notify_one();   //通知一个正在等待的线程
			return res;
		}
		inline ~ThreadPool() {   //销毁所有线程池中创建的线程
			{
				unique_lock<mutex> lock(queue_mutex);
				stop = true;   //设置线程状态
			}
			condition.notify_all();  //通知所有等待线程
			for (thread &worker : workers)   // 使所有异步线程转为同步执行, 此处循环为 c++11 新提供的循环语法 for(value:values)
				worker.join();
		}
private:
	vector<thread> workers;   //需要持续追踪线程老保证可以使用join
	queue<function<void()>> tasks;   //任务队列
	//同步相关
	mutex queue_mutex;    //互斥锁
	condition_variable condition; //互斥条件变量
	bool stop; //停止相关
};
#endif