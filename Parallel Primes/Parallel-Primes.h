#pragma once
#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <mutex>
#include <cmath>
#include <cstdint>


using namespace std;
using namespace std::chrono;

#pragma region Base Functions/Variables


int method_choice;
int range_max;
int thread_count;
int p_count;

mutex m;


 bool is_prime(int num)
{
	//handle edge cases
	if (num <= 1) return false;
	if (num == 2) return true;

	//check each divisor from 2 to square root of num, if any divide evenly, num is not prime
	for (int i = 2; i <= sqrt(num); i++)
	{
		if (num % i == 0)
		{
			return false;
		}
	}
	//otherwise return true
	return true;
}

#pragma endregion


#pragma region Mutex Functions

void mutex_thread(int start, int end)
{
	if (start < 2) start = 2; //In first partition, set start as 2 instead of 0

	int local = 0; //keep track of prime count within thread

	//loop through partition and increment for each found prime
 	for (int i = start; i <= end; ++i) {
		if (is_prime(i)) ++local;
	}

	//obtains mutex lock for write to global prime count
	lock_guard<mutex> g(m); //releases lock when out of scope
	p_count += local;
}

void mutex_counting(int max, int th_count) {
	if (th_count <= 0) th_count = 1; //handles invalid thread counts
	if (max < 2) { p_count = 0; return; } //no primes for ranges less than 2

	const int n = th_count; //Store th_count as "n" for easier readability
	const int chunk = (max + n - 1) / n; //finds partition sizes for each thread

	vector<thread> ts; //constructs vector to keep track of threads
	ts.reserve(n); //specifies capacity of vector

	for (int t = 0; t < n; ++t) {
		int start = t * chunk + 1; //calculates thread start by offset of previous chunks
		int end = min(max, (t + 1) * chunk); //calculates thread end by offset of previous chunks + the current chunk or the end of the search range
		if (start <= end) {
			ts.emplace_back(mutex_thread, start, end); //instantiate thread with start and end values passed in
		}
	}
	for (auto& th : ts) th.join(); //waits for all threads to finish work, joins
}

#pragma endregion


#pragma region Future Functions

void future_thread(int start, int end, promise<int>& promObj)
{
	//In first partition, set start as 2 instead of 0
	start = max(start, 2);

	int local = 0; //keep track of prime count within thread

	//loop through partition and increment for each found prime
	for (int i = start; i <= end; ++i) {
		if (is_prime(i)) ++local;
	}

	//once task is complete, the promise object value is set to the local prime count
	promObj.set_value(local);
}

void future_counting(int max, int th_count)
{
	
	if (max < 2) { p_count = 0; return; } //return prime count of 0 if less than two.

	const int n = th_count; //store th_count as "n" for easier readability
	const int chunk = (max + n - 1) / n; //finds partition sizes for each thread


	vector<thread> ts; //constructs vector to keep track of threads
	ts.reserve(n); //specifies capacity of threads vector

	vector<promise<int>> promises(th_count); //constructs vector to keep track of promise objects
	vector<future<int>> futures; //constructs vector to keep track of future objects
	futures.reserve(n); //specifies capacity of threads vector

	//Assigns each future object a promise object
	for (auto& p : promises)
	{
		futures.push_back(p.get_future());
	}


	for (int t = 0; t < n; ++t) {
		int start = t * chunk + 1; //calculates thread start by offset of previous chunks
		int end = std::min(max, (t + 1) * chunk); //calculates thread end by offset of previous chunks + the current chunk or the end of the search range
		if (start <= end) {
			ts.emplace_back(future_thread, start, end, ref(promises[t])); //instantiate thread, add it to threads vector, pass in start and end values of the partition and associated promise object
		}
	}
	for (auto& th : ts) th.join(); //waits for all threads to finish work, joins

	int total = 0; 
	for (auto& f : futures) total += f.get(); //promise objects had values set in threads, associated future objects get each of those values and add them to total
	p_count = total;

}

#pragma endregion


#pragma region Bitmap Sieve Class

/*
 * Summary of Approach:
 * A bitmap is used for memory and overhead efficiency; instantiated for odds only and index is used to compute corresponding value.
 * A vector of primes less than the square root of the search max is used as it contains all possible non-composite divisors
 * A sieve of Eratosthenes is used to efficiently calculate store prime values; CURRENTLY UNDERUTILIZED! THREADS CHECK OVERLAPPING VALUES
*/

class bitmap_sieve
{
public:

	
	//Packed bit array: bit i corresponds to integer i
	//Struct generated via ChatGPT
	struct dynamic_bitmap 
	{
		vector<uint64_t> map;
		size_t n_bits = 0; //number of odd entries

		explicit dynamic_bitmap(size_t n_bits_) : map((n_bits_ + 63) / 64, 0ULL), n_bits(n_bits_) {} //Constructor, initializer ensures correct rounding

		void set(size_t i) { map[i >> 6] |= (1ull << (i & 63)); }
		bool get(size_t i) const { return (map[i >> 6] >> (i & 63)) & 1ULL; }
	};

	explicit bitmap_sieve(int max)
 		:	N_(max),
 			n_odds_((N_ >= 3) ? ((size_t)N_ - 1) >> 1 : 0),
 			map_(n_odds_) {}

	static inline size_t j_from_val(int v) { return (size_t)(v - 3) >> 1; }

	//Primes less than square root N (max) are few enough to be added to a standard int vector using brute force prime checking method.
	void store_base_primes(int n)
	{
		base_primes.clear();
		if (N_ < 2) return;
		
		for (int i = 2; i <= floor(sqrt(n)); i++)
		{
			if (is_prime(i)) base_primes.push_back(i);
		}
	}

 	bool check_prime_divisibility(int num)
	{
		for (auto var : base_primes)
		{
			if (1LL * var * var > num) break;
			if (num == var) return false;
			if (num % var == 0) return true;
		}
		return false;
	}

	void bitmap_thread(int start, int end)
	{
		for (int i = start; i <= end; i += 2)
		{
			if (check_prime_divisibility(i)) map_.set(j_from_val(i));
		}
	}

	void bitmap_p_update(int max, int th_count)
	{
		if (th_count <= 0) th_count = 1;
		if (max < 2) return ;

		store_base_primes(max);

		const int n = th_count;
		const int chunk = (max + n - 1) / n;

		vector<thread> ts;
		ts.reserve(n);

		for (int t = 0; t < n; ++t) {
			int start = t * chunk + 1;
			int end = std::min(max, (t + 1) * chunk);
			if (start <= end) {
				ts.emplace_back([this, start, end] ()
				{
					this->bitmap_thread(start, end);
				});
			}
		}
		for (auto& th : ts) th.join();
	}

	void determine_m_primes() {
		p_count = (N_ >= 2) ? 1 : 0;  // include prime 2
		for (size_t j = 0; j < n_odds_; ++j)
			if (!map_.get(j)) ++p_count; // 0-bit means prime among odds
	}

	void print_bm_primes ()
	{
		static int line_len = 25;
		int cur_line = 0;
		for (size_t j = 0; j < n_odds_; ++j)
		{
			if (!map_.get(j)) cout << j << " ", cur_line++; // 0-bit means prime among odds
			if (cur_line >= line_len) cout << '\n', cur_line = 0;
		}
	}

	private:
		int N_{ 0 };
		size_t n_odds_{ 0 };
		dynamic_bitmap map_;
		vector<int> base_primes;

};




#pragma endregion

