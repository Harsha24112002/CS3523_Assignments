#include<iostream>
#include<fstream>
#include<pthread.h>
#include<atomic>
#include<random>
#include<chrono>
#include<unistd.h>
// Global Variables
int num_threads ; // Numeber of threads to be created
int k; // Number of times each thread requests for CS
float l1 ,l2; // Lambda1, Lambda2 values for simulating the times taken in CS and Remainder Section
std::atomic<int> lock(0);// Atomic lock variable
std::ofstream out; 
float wait_time, worst_case_wait_time; // Total Wait time and worst case wait time

/**
 * @brief Returns the time(hh:mm:ss) as a string, given time_t
 * 
 * @param cur_time 
 * @return std::string 
 */
std::string gettime(time_t& time)
{
	std::string format = ctime(&time);
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
	// Thread id
	long param = (long) args;
	// Random variable generator for exponential distribution to be used for simulation of CS and Remainder sections
	int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::exponential_distribution<double> exp_dist1(1/l1);
	std::exponential_distribution<double> exp_dist2(1/l2);
	//K calls to CS
	for(unsigned i=0;i<k;i++)
	{
		//Requesting CS
		time_t req_time = time(NULL);
		// Busy wait 
		while(true)
		{
			// Resetting the test value as compare exchange weak may change its value 
			// Declaring the comp variable inside so that each thread have its own comp,
			// So even if it the test value(comp) is changed after the compare exchange weak execution, 
			// the other threads won't have an affect
			int comp = 0; 
			if(lock.compare_exchange_weak(comp,1)==true) break; 
		}
		out<<(i+1)<<"th CS request at " << gettime(req_time) << " by "<<"Thread "<< (param+1) << std::endl;
		
		//Entry Section
		
		time_t enter_time = time(NULL);
		out<<(i+1)<<"th CS entered at " << gettime(enter_time) << " by "<<"Thread "<< (param+1) << std::endl;
		
		// Calculationg total wait time and worst case wait time

		int current_wait_time = enter_time - req_time;
		wait_time += current_wait_time;
		worst_case_wait_time = (current_wait_time>worst_case_wait_time)?current_wait_time:worst_case_wait_time;
		
		// Simulation of CS
		sleep(exp_dist1(generator));

		//Exit Section

		time_t exit_time = time(NULL);
		out<<(i+1)<<"th CS exited at " << gettime(exit_time) << " by "<<"Thread "<< (param+1) << std::endl;
		lock = 0; // Freeing the lock just after exiting CS
		

		//Simulation of Remainder Section
		sleep(exp_dist2(generator));
	}
	pthread_exit(0);

}
int main()
{
	// Taking input from file
	std::ifstream inp;
	inp.open("inp-params.txt");
	inp >> num_threads >> k >> l1 >>l2;
	wait_time = 0;
	worst_case_wait_time = 0;

	pthread_t threads[num_threads];
	pthread_attr_t attributes[num_threads];
	
	out.open("CAS-Log.txt");
	out << "CAS ME Output\n";
	// Creating threads
	for(long i=0;i<num_threads;i++)
	{
		pthread_attr_init(&attributes[i]);
		pthread_create(&threads[i],&attributes[i],testCS,(void *)(i));
	}
	// Joining threads
	for(long i=0;i<num_threads;i++)
	{
		pthread_join(threads[i],NULL);
	}

	// Writing average waiting time and worst case waiting time to output
	out<<"Average Waiting time :" << wait_time/(num_threads*k) <<std::endl;
	out<<"worst case waiting time : " << worst_case_wait_time << std::endl;

	//Closing the input output files
	out.close();
	inp.close();
}	
