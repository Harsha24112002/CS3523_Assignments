// Libraries
#include<iostream>
#include<fstream>
#include<pthread.h>
#include<atomic>
#include<random>
#include<chrono>
#include<unistd.h>

// Global variables
int num_threads; // number of threads
int k; // Number of calls to CS
float l1,l2; // Lambda1, Lambda2
int* waiting; // waiting array, to store whether it is waiting or not to enter the CS
std::atomic<int> lock(0); // Atomic lock
float wait_time, worst_case_wait_time;	// waiting times 
std::ofstream out;	// output file

/**
 * @brief Returns the time(hh:mm:ss) as a string, given time_t
 * 
 * @param cur_time 
 * @return std::string 
 */
std::string gettime(time_t& time)
{
	std::string formatted = ctime(&time);
	return formatted.substr(11,8);

}

/**
 * @brief Thread function for testing Mutual exclusion in critical section
 * 
 * @param args 
 * @return void* 
 */
void* testCS(void* args)
{
	// For keeping track of thread ids	
	long param = (long) args;

	// Exponentinal random variable generator for simulating critcal section and remainder section
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	// Exponential distribution for lambda1, lambda2
	std::exponential_distribution<double> exp_dist1(1/l1);
	std::exponential_distribution<double> exp_dist2(1/l2);

	// Each thread request CS k times
	for(unsigned i=0;i<k;i++)
	{
		// Request
		time_t req_time = time(NULL);

		waiting[param] = true;
		int key = 1;
		// Entry Section
		while(waiting[param] && key == 1)
		{
			// Resetting the test value as compare exchange weak may change its value 
			// Declaring the comp variable inside so that each thread have its own comp,
			// So even if it the test value(comp) is changed after the compare exchange weak execution, 
			// the other threads won't have an affect
			int comp = 0;
			key = 1 - lock.compare_exchange_strong(comp,1);
		}
		waiting[param] = false;
		out<<(i+1)<<"th CS request at " << gettime(req_time) << " by "<<"Thread "<< (param+1) << std::endl;
	
		time_t enter_time = time(NULL);
		out<<(i+1)<<"th CS entered at " << gettime(enter_time) << " by "<<"Thread "<< (param+1) << std::endl;
		
		// Calculation of wait time and worst_case wait time
		int current_wait_time = (enter_time - req_time);
		worst_case_wait_time = (current_wait_time>worst_case_wait_time)?current_wait_time:worst_case_wait_time;
		wait_time += current_wait_time;
		
		//Simulation of CS
		sleep(exp_dist1(generator));
		
		// Exit Section
		time_t exit_time = time(NULL);
		out<<(i+1)<<"th CS exited at " << gettime(exit_time) << " by "<<"Thread "<< (param+1) << std::endl;
		
		// Following a cyclic order to ensure bounded waiting
		int j = (param+1)%num_threads;
		while((j!=param) && !waiting[j])
		{
			j = (j+1)%num_threads;
		}

		// If no thread is waiting, free the lock, 
		// else allow the waiting thread to enter the CS
		if(j==param)
		{
			lock = 0;
		}
		else
		{
			waiting[j] = false;
		}

		//Simulation of Remainder Section
		sleep(exp_dist2(generator));

	}
	pthread_exit(0);

}
int main()
{
	// input file
	std::ifstream inp;
	inp.open("inp-params.txt");
	inp >> num_threads >> k >>l1 >>l2;
	
	// Initializing the wait time, worst case wait time
	wait_time = 0;
	worst_case_wait_time = 0;

	waiting = (int*)malloc(sizeof(int)*num_threads);

	pthread_t threads[num_threads];
	pthread_attr_t attributes[num_threads];

	out.open("BoundedCas-log.txt");
	out << "Bounded CAS ME Output:\n";
	
	for(long i=0;i<num_threads;i++)
	{
		waiting[i] = false;
	}
	// Creating the threads
	for(long i=0;i<num_threads;i++)
	{
		pthread_attr_init(&attributes[i]);
		pthread_create(&threads[i],&attributes[i],testCS,(void * )i);
	}
	// Joining the threads
	for(long i=0;i<num_threads;i++)
	{
		pthread_join(threads[i],NULL);
	}
	// Freeing the memory allocated for waiting array
	free(waiting);
	// Writing the average waiting time and worst case waiting time to the output
	out << "Average Waiting time : "<< wait_time/(num_threads*k) << std::endl;
	out << "Worst Case waiting time : "<<worst_case_wait_time << std::endl;
	// Closing the input and output
	out.close();
	inp.close();
}	
