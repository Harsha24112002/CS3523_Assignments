// Libraries
#include<iostream>
#include<fstream>
#include<pthread.h>
#include<atomic>
#include<random>
#include<chrono>
#include<unistd.h>

// Global variables
int num_threads; // Nuumber of threads
int k; // k calls to CS
float l1,l2 ; // lambda1, lambda2 means for exp distribution
std::atomic_flag lock = ATOMIC_FLAG_INIT;// atomic lock for test and set
float wait_time, worst_case_wait_time;
std::ofstream out; // output file

/**
 * @brief Returns the time(hh:mm:ss) as a string, given time_t
 * 
 * @param cur_time 
 * @return std::string 
 */
std::string gettime(time_t& cur_time)
{
	std::string format = ctime(&cur_time);
	return format.substr(11,8);
}

/**
 * @brief Thread function for testing Mutual exclusion in critical section
 * 
 * @param args 
 * @return void* 
 */
void* testCS(void* args)
{
	// Getting the paramater passed to use as thread id	
	int param = (long) args;
	
	// Exponential distributions for simulations of cs and remainder sections
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	// Given l1,l2 are means so passing 1/l1, 1/l2
	std::exponential_distribution<double> exp_dist1(1/l1); 
	std::exponential_distribution<double> exp_dist2(1/l2);
	// K calls to CS
	for(unsigned i=0;i<k;i++)
	{
		//Request
		time_t req_time = time(NULL);
	
		while(lock.test_and_set()){}; // Busy-wait using test and set
		
		// Entry section 
		out<<(i+1)<<"th CS request at " << gettime(req_time) << " by "<<"Thread "<< (param+1) << std::endl;
		time_t enter_time = time(NULL);
		out<<(i+1)<<"th CS entered at " << gettime(enter_time) << " by "<<"Thread "<< (param+1) << std::endl;
		
		// Calculating total waiting time and worst case wait time
		int current_wait_time = (enter_time - req_time);
		worst_case_wait_time = (current_wait_time>worst_case_wait_time)?current_wait_time:worst_case_wait_time;
		wait_time += current_wait_time;
		
		//Simulation of Critcal Section
		sleep(exp_dist1(generator));

		time_t end_time = time(NULL);
		out<<(i+1)<<"th CS exited at " << gettime(end_time) << " by "<<"Thread "<< (param+1) << std::endl;
		
		// Releasing the lock
		lock.clear();

		//Simulation of Remainder section
		sleep((exp_dist2(generator)));
	}
	pthread_exit(0);

}
int main()
{
	// Getting input from file
	std::ifstream inp;
	inp.open("inp-params.txt");
	inp >> num_threads >> k >> l1 >>l2;

	wait_time = 0;
	worst_case_wait_time = 0;

	pthread_t threads[num_threads];
	pthread_attr_t attributes[num_threads];
	
	out.open("TAS-Log.txt");
	out<<"TAS ME Output\n";
	// Creating threads 
	for(long i=0;i<num_threads;i++)
	{
		pthread_attr_init(&attributes[i]);
		pthread_create(&threads[i],&attributes[i],testCS,(void*)i);
	}
	//Joining threads
	for(long i=0;i<num_threads;i++)
	{
		pthread_join(threads[i],NULL);
	}

	// Outputting the average waittime of each process and worst case wait time
	out<<"Average Waiting time : "<<wait_time/(num_threads*k) <<"\n";
	out<<"Worst case wait time : " << worst_case_wait_time<<std::endl;

	// Closing input and output	
	out.close();
	inp.close();
}	
