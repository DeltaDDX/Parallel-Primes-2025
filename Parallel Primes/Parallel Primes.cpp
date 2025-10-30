
//Collaborators: ChatGPT

#include <chrono>
#include <string>
#include "Parallel-Primes.h"


int main(int argc, char* argv[])
{
	if (argc >= 3)
	{
		system_clock::time_point start = system_clock::now();

		bitmap_sieve bm(atoi(argv[2]));
		bm.bitmap_p_update(atoi(argv[2]), atoi(argv[1]));
		bm.determine_m_primes();

		auto end = system_clock::now();
		auto diff = duration_cast<milliseconds> (end - start).count();

		if (argv[3] == "-p")
		{
			bm.print_bm_primes();
		}

		cout << '\n' << "Number of Primes in Range: " << p_count << "\n";
		cout << '\n' << "Total Time Taken: " << diff << " Milliseconds\n";
	}

	else
	{
		cout << "Select Threading Method: 1 - Mutex, 2 - Future, 3 - Bitmap Sieve (WORK IN PROGRESS)\nType: ";
		cin >> method_choice;
		cout << "Input search maximum (< 20,000,000): ";
		cin >> range_max;
		cout << "Input thread count (Check device specification for recommended max): ";
		cin >> thread_count;

		system_clock::time_point start = system_clock::now();

		switch (method_choice)
		{
		default:
			mutex_counting(range_max, thread_count);
			break;
		case 2:
			future_counting(range_max, thread_count);
			break;
		case 3:
			bitmap_sieve bm(range_max);
			bm.bitmap_p_update(range_max, thread_count);
			bm.determine_m_primes();
			break;
		}


		auto end = system_clock::now();
		auto diff = duration_cast<milliseconds> (end - start).count();

		cout << "Number of Primes in Range: " << p_count << "\n";
		cout << "Total Time Taken: " << diff << " Milliseconds\n";
	}
	

	return 0;
}



