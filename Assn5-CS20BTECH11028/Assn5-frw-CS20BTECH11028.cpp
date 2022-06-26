// Implementation of Fair Readers and Writers
#include<iostream>
#include<fstream>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<random>
#include<chrono>
#include<sys/time.h>

#define MICRO 1000000
#define MILLI 1000
int num_threads_writer, num_threads_reader; // Number of writer threads and reader threads
int kw,kr; // Number of requests into the CS
sem_t mutex; // semaphore used for CS in readers to ensure ME while incrementing and decrementing reader count
sem_t rw_mutex; // Semaphore used to maintain ME between a writer and other readers or writers
sem_t queue_mutex; // Semaphore used to maintain the order and to get fairness
float mucs,mrem; // Averages of the exp distributed variables
// Total and worst case waiting times of readers and writers
float reader_waittime, writer_waittime, reader_worst_case_wait_time, writer_worst_case_wait_time;
int reader_count = 0;	// Counting number of reader threads are in the CS.
std::ofstream out,times; // Outputting the log and waiting times 
int seed = std::chrono::system_clock::now().time_since_epoch().count();

// Exponential distribution for simulating CS and Remainder sections
std::default_random_engine generator(seed);
// Exponential distribution for lambda1, lambda2
std::exponential_distribution<double> exp_dist1;
std::exponential_distribution<double> exp_dist2;
/**
 * @brief Get the time as string
 * 
 * @param timer the time_t object for which the string is to be generated
 * @return std::string formatted substring which gives the current time in hh::mm::ss
 */
std::string get_time(time_t& timer)
{
	std::string formatted;
	formatted = ctime(&timer);
	return formatted.substr(11,8);

}
/**
 * @brief Get the duration between two timevals in microseconds
 * 
 * @param start start time 
 * @param end end time
 * @return float 
 */
float getduration(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec)*MICRO + (end.tv_usec - start.tv_usec);
}
/**
 * @brief Thread Function for writer threads
 * 
 * @param args 
 * @return void* 
 */
void* Writer(void* args)
{
	// Storing the thread id
	long thread_id = (long)args;
	// kw requests to CS
	for(int i=0;i<kw;i++)
	{
		// Request to CS
		struct timeval req_time;
		// To load the current time into the req time
		gettimeofday(&req_time,NULL);

		sem_wait(&queue_mutex); // To preserve the order
		sem_wait(&rw_mutex); 
		sem_post(&queue_mutex); 

		// Critical Section
		struct timeval enter_time;
		// To load the current time into the enter time
		gettimeofday(&enter_time,NULL);
		out<< i+1 << "th CS request by Writer thread " << (thread_id + 1) << " at time "<<get_time(req_time.tv_sec) << std::endl;
		
		out<< i+1 << "th CS entered by Writer thread " << (thread_id + 1) << " at time "<<get_time(enter_time.tv_sec) << std::endl;

		// Calculation of wait times
		float waittime = getduration(req_time,enter_time);
		writer_waittime += waittime;
		writer_worst_case_wait_time = (waittime>writer_worst_case_wait_time)?(waittime):writer_worst_case_wait_time;

		// Simulation of CS
		usleep(exp_dist1(generator)*MILLI);
		struct timeval exit_time;
		gettimeofday(&exit_time,NULL);
		// To load the current time into the exit time
		out<< i+1 << "th CS exited by Writer thread " << thread_id + 1 << " at time "<<get_time(exit_time.tv_sec) << std::endl;

		sem_post(&rw_mutex);
		// Remainder Section
		usleep(exp_dist2(generator)*MILLI);
	}

	pthread_exit(0);
}
/**
 * @brief Thread function for reader threads
 * 
 * @param args 
 * @return void* 
 */
void* Reader(void* args)
{
	// Storing the thread ids
	long thread_id = (long)args;

	// kr requests 	
	for(int i=0;i<kr;i++)
	{
		// Reuest to CS
		struct timeval req_time;
		// To load the current time into the req time
		gettimeofday(&req_time,NULL);
	
		sem_wait(&queue_mutex);
		sem_wait(&mutex);
		reader_count++; // incrementing the reader count
		if(reader_count == 1)
		{
			sem_wait(&rw_mutex); // Waiting on rw_mutex
		}

		sem_post(&queue_mutex);

		struct timeval enter_time;
		// To load the current time into the enter time
		gettimeofday(&enter_time,NULL);
		out<< i+1 << "th CS request by Reader thread " << thread_id + 1 << " at time "<<get_time(req_time.tv_sec) << std::endl;

		// Calculation of waiting times
		float waittime = getduration(req_time,enter_time);
		reader_waittime += waittime;		
		reader_worst_case_wait_time = waittime>reader_worst_case_wait_time?waittime:reader_worst_case_wait_time;
		
		out<< i+1 << "th CS entered by Reader thread " << thread_id + 1 << " at time "<<get_time(enter_time.tv_sec) << std::endl;
		sem_post(&mutex); // Signalling the mutex once the reader count is incremented
		// Simulation of CS
		usleep(exp_dist1(generator)*MILLI);

		sem_wait(&mutex);
		reader_count--; // Decrementing the thread count
		struct timeval exit_time;
		// To load the current time into the exit time
		gettimeofday(&exit_time,NULL);
		out<< i+1 << "th CS exited by Reader thread " << thread_id + 1 << " at time "<<get_time(req_time.tv_sec) << std::endl;
		if(reader_count == 0)
		{
			sem_post(&rw_mutex); // If no reader in CS signal the rw_mutex
		}
		sem_post(&mutex);
		//Remainder Section
		usleep(exp_dist2(generator)*MILLI);
	}
	pthread_exit(0);
}

int main()
{
	// Initializing the wait times
	reader_worst_case_wait_time = 0;
	writer_worst_case_wait_time = 0;
	reader_waittime = 0;
	writer_waittime = 0;
	// Input file
	std::ifstream inp;
	inp.open("inp-params.txt");
	inp >> num_threads_reader >> num_threads_writer >> kr >> kw >> mucs >> mrem;

	// Initializing Exponential distributions 
	exp_dist1 = std::exponential_distribution<double>(1/mucs);
	exp_dist2 = std::exponential_distribution<double>(1/mrem);

	// Intializing semaphores with 1
	sem_init(&mutex,0,1);
	sem_init(&rw_mutex,0,1);
	sem_init(&queue_mutex,0,1);

	// Writer threads
	pthread_t writerthreads[num_threads_writer];
	pthread_attr_t writer_attr[num_threads_writer];
	
	// Reader threads
	pthread_t readerthreads[num_threads_reader];
	pthread_attr_t reader_attr[num_threads_reader];

	out.open("FairRW-log.txt");
	times.open("Average_time.txt", std::ios_base::app | std::ios_base::in);
	// Creating the writer threads
	for(long i=0;i<num_threads_writer;i++)
	{
		pthread_attr_init(&writer_attr[i]);
		pthread_create(&writerthreads[i],&writer_attr[i],Writer,(void*)i);
	}
	// Creating the reader threads
	for(long i=0;i<num_threads_reader;i++)
	{
		pthread_attr_init(&reader_attr[i]);
		pthread_create(&readerthreads[i],&reader_attr[i],Reader,(void*)i);
	}
	// Joining writer threads
	for(long i=0;i<num_threads_writer;i++)
	{
		pthread_join(writerthreads[i],NULL);
	}
	// Joining reader threads
	for(long i=0;i<num_threads_reader;i++)
	{
		pthread_join(readerthreads[i],NULL);
	}
	// Printing wait times to output file
	if(times.is_open())
	{
		times << "-- Waiting times in Fair RW -- "<<std::endl;
		times << "Writer thread average wait time : "<< writer_waittime/(1000*(num_threads_writer)*kw)<<std::endl;
		times << "Writer thread worst case wait time : "<<writer_worst_case_wait_time/1000<<std::endl;
		times << "Reader thread average wait time : "<< reader_waittime/(1000*(num_threads_reader)*kr)<<std::endl;
		times << "Reader thread worst case wait time : "<< reader_worst_case_wait_time/1000 <<std::endl;
	}
	// Closing the input and outputs
	times.close();
	out.close();
	inp.close();
	return 0;
}