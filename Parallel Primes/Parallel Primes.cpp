#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <mutex>
#include <chrono>
#include <cmath>

using namespace std;
using namespace std::chrono;

namespace
{
	int method_choice;
	int range_max;
	int thread_count;
	int p_count;

	mutex m;
}

bool isPrime(int num)
{
	if (num <= 1) return false;
	if (num == 2) return true;

	for (int i = 2; i <= sqrt(num); i++)
	{
		if (num % i == 0)
		{
			return false;
		}
	}
	return true;
}

void example_thread(promise<int>* promObj)
{
	cout << "Inside Thread" << "\n";
	promObj->set_value(35);
}

void example_thread_start()
{
	promise<int> promise_obj;
	future<int> future_obj = promise_obj.get_future();
	thread th(example_thread, &promise_obj);
	cout << future_obj.get() << "\n";
	th.join();
}


void mutex_thread(int start, int end) {
	// skip 0/1
	if (start < 2) start = 2;

	int local = 0;
	for (int i = start; i <= end; ++i) {
		if (isPrime(i)) ++local;
	}
	lock_guard<mutex> g(m);
	p_count += local;
}

void mutex_counting(int max, int th_count) {
	if (th_count <= 0) th_count = 1;
	if (max < 2) { p_count = 0; return; }

	const int n = th_count;
	const int chunk = (max + n - 1) / n;

	vector<thread> ts;
	ts.reserve(n);

	for (int t = 0; t < n; ++t) {
		int start = t * chunk + 1;
		int end = std::min(max, (t + 1) * chunk);
		if (start <= end) {
			ts.emplace_back(mutex_thread, start, end);
		}
	}
	for (auto& th : ts) th.join();
}
	
int main()
{
	cout << "Select Threading Method: 1 - Mutex, 2 - Future (WORK IN PROGRESS), 3 - CUDA (WORK IN PROGRESS)\nType: ";
	cin >> method_choice;
	cout << "Input search maximum: ";
	cin >> range_max;
	cout << "Input thread count (Max of 10): ";
	cin >> thread_count;
	
	system_clock::time_point start = system_clock::now();

	switch (method_choice)
	{
	default:
		mutex_counting(range_max, thread_count);
		break;
	case 2:

		break;
	case 3:

		break;
	}


	auto end = system_clock::now();
	auto diff = duration_cast<milliseconds> (end - start).count();

	cout << "Number of Primes in Range: " << p_count << "\n";
	cout << "Total Time Taken: " << diff << " Milliseconds\n";

	return 0;
}

