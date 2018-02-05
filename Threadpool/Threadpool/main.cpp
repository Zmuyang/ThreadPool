#include <iostream> 
#include <vector>   
#include <string>   
#include <future>   
#include <thread>   
#include <chrono>   
#include "ThreadPool.hpp"
using namespace std;

int main() {
	ThreadPool pool(4); //����һ���ܹ�����ִ���ĸ��̵߳��̳߳�
	vector<future<string>> results;  //��������ִ���̵߳Ľ���б�
	for (int i = 0; i < 8; ++i) {   //�����˸���Ҫִ�е��߳�����
		results.emplace_back(  //����������ִ�еķ���ֵ��ӵ�����б���
			pool.enqueue([i] {  //����ӡ������ӵ��̳߳��в�����ִ��
			cout << "hello"<< endl << i << endl;
			this_thread::sleep_for(chrono::seconds(1)); //��һ������󣬸��̻߳�ȴ�һ����
			cout << "World" <<endl<< i << endl; //���������������ִ�����
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