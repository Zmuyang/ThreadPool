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
	inline ThreadPool(size_t threads) : stop(false)   //���̳߳��д���threads�������߳�
	{
		for(size_t i=0;i<threads;++i)   //���������߳�
			workers.emplace_back([this]   //�˴���lambda���ʽ����this�����̳߳�ʵ��
		{
			for (;;)  //ѭ��������ٻ���
			{
				function<void()> task; {   // ���庯�����������, �洢����ķ�������Ϊ void ������Ϊ�յĺ���
					unique_lock<mutex> lock(this->queue_mutex);   //����������
					this->condition.wait(lock,   //������ǰ�̣߳�֪��condition_variable������
						[this] {return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())     // �����ǰ�̳߳��Ѿ������ҵȴ��������Ϊ��, ��Ӧ��ֱ�ӷ���
						return;
					task = move(this->tasks.front());   // �������������еĶ���������Ϊ��Ҫִ�е��������
					this->tasks.pop();
				}
				task();   //ִ�е�ǰ����
			}
		});
	}
	template<typename F,typename... Args >  //���̳߳��������߳�
	auto enqueue(F&& f, Args&&... args)    //F&&������ֵ����
		->future<typename result_of<F(Args...)>::type> {
			using return_type = typename  result_of<F(Args...)>::type;   //ָ�����񷵻�����
			auto task = make_shared<packaged_task<return_type()>>(   //��õ�ǰ����
				bind(forward<F>(f),forward<Args>(args)...)
				);
			future<return_type> res = task->get_future();   // ��� std::future �����Թ�ʵʩ�߳�ͬ��
			{
				unique_lock<mutex> lock(queue_mutex);
				if (stop)    //��ֹ���̳߳�ֹͣ������µ��߳�
				{
					throw runtime_error("enqueue on stopped ThreadPool");
				}
				tasks.emplace([task] {(*task)(); });   //���߳���ӵ�ִ�����������
			}
			condition.notify_one();   //֪ͨһ�����ڵȴ����߳�
			return res;
		}
		inline ~ThreadPool() {   //���������̳߳��д������߳�
			{
				unique_lock<mutex> lock(queue_mutex);
				stop = true;   //�����߳�״̬
			}
			condition.notify_all();  //֪ͨ���еȴ��߳�
			for (thread &worker : workers)   // ʹ�����첽�߳�תΪͬ��ִ��, �˴�ѭ��Ϊ c++11 ���ṩ��ѭ���﷨ for(value:values)
				worker.join();
		}
private:
	vector<thread> workers;   //��Ҫ����׷���߳��ϱ�֤����ʹ��join
	queue<function<void()>> tasks;   //�������
	//ͬ�����
	mutex queue_mutex;    //������
	condition_variable condition; //������������
	bool stop; //ֹͣ���
};
#endif