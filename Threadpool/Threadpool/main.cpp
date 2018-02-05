#include <iostream> 
#include <vector>   
#include <string>   
#include <future>   
#include <thread>   
#include <chrono>   
#include "ThreadPool.hpp"
using namespace std;

int main() {
	ThreadPool pool(4); //创建一个能够并发执行四个线程的线程池
	vector<future<string>> results;  //创建并发执行线程的结果列表
	for (int i = 0; i < 8; ++i) {   //启动八个需要执行的线程任务
		results.emplace_back(  //将并发任务执行的返回值添加到结果列表中
			pool.enqueue([i] {  //将打印任务添加到线程池中并并发执行
			cout << "hello"<< endl << i << endl;
			this_thread::sleep_for(chrono::seconds(1)); //上一行输出后，该线程会等待一秒钟
			cout << "World" <<endl<< i << endl; //随后继续输出并返回执行情况
			return string("---thread") + to_string(i) + string(" finished.---");
		})
		);
	}
	for (auto && result : results)
		cout << result.get() << ' ';
	cout << endl;
	system("pause");
	return 0;
}