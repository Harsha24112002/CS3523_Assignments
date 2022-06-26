#include<iostream>
#include<queue>
#include<set>
#include<fstream>
#define contextswitch 0.01f
#define infinite 10000000
/**
 * Class for each process which stores the remaining time, incoming time, processing time, period,
 *  waiting time, number of iterations,isprempted or not etc
 * 
 */
class process{
	public:
	/**
	 * @brief Resets the process data to be used for next iteration
	 * 
	 * @return 1 if process is available for next iteration, 2 if all iterations completed 
	 */
	int reset()
	{
		this->wait_time = this->wait_time + (this->outtime - (this->intime + this->proc_time - this->rem_time));
		if(round == k+1)
		{
			return 2;
		}
		this->intime = this->intime + period;
		this->rem_time = this->proc_time;
		this->ispreempted = false;
		this->round++;
		return 1;
	}
	//Constructor for initialization
	process(int id,int period,int proc_time,int k)
	{
		this->id = id;
		this->period = period;
		this->proc_time = proc_time;
		this->deadline = this->period;
		this->k = k;
		this->intime = 0;
		this->rem_time = proc_time;
		this->round = 1;
		this->wait_time = 0;
		this->ispreempted = false;
	}
	int id;
	int period;
	int deadline;
	int proc_time;
	int k;
	int round;

	int wait_time;
	int rem_time;
	int intime;
	int outtime;
	bool ispreempted;
};
/**
 * @brief Returns the process with higher priority, i.e., earliest deadline in case of EDF
 * 
 * @param processes vector of processes from which the process with higher priority is needed
 * @return process* 
 */
process* gethighprior(std::vector<process*> processes)
{
	int min = infinite;
	int minrem = infinite;
	int index = -1; 
	std::vector<int> indices;
	for(int i=0;i<processes.size();i++)
	{ // Finding the process with earliest deadline
		if(min >= processes[i]->deadline+ processes[i]->intime)
		{
			min = processes[i]->deadline + processes[i]->intime;
			index = i;
			indices.push_back(i);
		}
	}
	// If there was no process in the vector then return NULL
	if(index==-1)
	{
		return NULL;
	}
	else
	{
		return processes[index];
	}
}
/**
 * @brief Gives the index of a process, in a vector of processes
 * 
 * @param processes 
 * @param current 
 * @return int , index of the process if present, else -1
 */
int getindex(std::vector<process*> processes,process* current)
{
	int index = -1;
	for(unsigned i=0;i<processes.size();i++)
	{
		if(processes[i]==current)
		{
			index = i;
			break;
		}
	}
	return index;
}
/**
 * @brief Returns the number of 1s in the given vector, 
 * used for checking number of processes finished completely0
 * 
 * @param a vector<int> to be checked
 * @return int 
 */
int getcount(std::vector<int> a)
{
	int count = 0;
	for(unsigned i=0;i<a.size();i++)
	{
		if(a[i]==1)
		count++;
	}
	return count;
}
int main()
{
	std::ifstream inp; //input
	//Input error check
	if(!inp)
	{
		std::cout<<"Error opening input file\n";
		return -1;
	}
	inp.open("inp-params.txt");
	// Taking input from file
	int n;
	std::vector<process*> processes;
	inp >> n;
	int tot = 0;
	for(unsigned i=0;i<n;i++)
	{
		int pid,proctime,period,k;
		inp >> pid >> proctime >> period>>k;
		tot += k;
		process* p = new process(pid,period,proctime,k);
		processes.push_back(p);
	}
	//Outputs
	std::ofstream log,stats;
	//Output error checks
	log.open("EDF-log.txt");
	if(!log)
	{
		std::cout<<"Error opening log file\n";
		return 1;
	}
	stats.open("EDF-stats.txt");
	if(!stats)
	{
		std::cout<<"Error opening stats file\n";
	}
	//Currently running processes are stored here
	std::vector<process*> edfqueue;
	//To check whether a process is completed or not
	std::vector<int> fin (n,0);
	//Initializing the running processes
	for(int i=0;i<n;i++)
	{
		edfqueue.push_back(processes[i]);
		log << "Process " << processes[i]->id << " entered the system at time : 0, Deadline: "<<processes[i]->deadline << ", period: "<< processes[i]->period << ", processing time: "<<processes[i]->proc_time << std::endl;
	}
	// Starting the time at 1
	int time = 1;
	//Number of deadline misses and Successful completions
	int deadlinemisses = 0;
	int successfullcomp = 0;
	//Running until all iterations of all processes are processed
	while(getcount(fin)!=n)
	{
		process* current = gethighprior(edfqueue); // Getting process with highest priority
		bool ispreempted = false;
		// Checking if the current process has completed all iterations or not
		// If completed remove it and then start next high priority process
		while(current!=NULL && current->round>current->k)
		{
			if(fin[current->id-1]!=1)
			{
				fin[current->id-1]=1;
			}
			edfqueue.erase(getindex(edfqueue,current) + edfqueue.begin());
			current = gethighprior(edfqueue);
		}
		
		//If no processes entered in CPU, wait for any process to enter
		if(current!=NULL && !edfqueue.empty())
		{
			// Resuming if preempted else starting fresh
			if(current->ispreempted==true)
			{
				log<<"P"<<current->id<<" resumed at time "<< time <<std::endl;
			}
			else
			{
				log<<"P"<<current->id<<" started at time "<< time <<std::endl;
			}
			// Flag for the current running process deadline miss 
			bool currentmisseddeadline = false;
			// Flag to check if the current process completed at this time
			bool currentcompleted = false;
			/**
			 * While a process is running, we check all the processes whether they are coming,
			 * deadline missed etc
			 * Checking whether the running process is finished, newly added process pre-empted the  
			 * already running one etc
			 */
			while(current->rem_time>0 && (current->round)<=(current->k))
			{
				for(unsigned i =0;i<n;i++)
				{
					//If all iterations are not completed
					if(processes[i]->round <= processes[i]->k)
					{
						if(time%processes[i]->period == 0)
						{
							bool isdead =false;
							if(processes[i]->rem_time>0)
							{
								// Flag for checking if the current process is finished or missed its deadline
								// Anyway it wouldn't be changed for processes other than the current process 
								bool flag = false; 
								
								if(processes[i]==current)
								{
									if(current->rem_time==1)
									{
										// The current process is ending at itd deadline
										current->rem_time--;
										log<<"P"<<processes[i]->id<<" finished at time "<<time<<"\n";
										flag = true;	
										currentcompleted = true;
										// If current process completed we delete it from the edfqueue
										edfqueue.erase(getindex(edfqueue,current) + edfqueue.begin());
										successfullcomp++;
									}
									else
									{
										// The current process missed deadline
										currentmisseddeadline = true;
									}
								}
								if(!flag)
								{
									log<<"P"<<processes[i]->id<<" missed its deadline at time "<< time<<"\n";
									deadlinemisses++;
									isdead = true;
								}
								// Storing the termination time of the process
								processes[i]->outtime = time;
							}
							// Resetting the process which missed deadline or entering cpu
							int notify=processes[i]->reset();

							if(isdead && notify!=2)
							{
								continue;
							}
							if(notify!=2) // If all iterations are not completed push it into the edfqueue
							{
								edfqueue.push_back(processes[i]);
							}
						}	
						
					}
					else // If all iterations are completed
					{
						if(fin[processes[i]->id-1]!=1)
						{
							fin[processes[i]->id-1] = 1; // For checking if a process is completed or not
						}
					}
				}
				//Checking if all process completed
				if(getcount(fin)==n)
				{
					break;
				}
				// If current process completed, as already we resetted it before,
				// increment time and break
				if(currentcompleted)
				{
					time++;
					break;
				}
				// If the currently running process missed its deadline, we increment time, and start 
				if(currentmisseddeadline)
				{
					time++;
					break;
				}
				time++;
				// If the current process is not of highpriority after adding new incoming processes
				if(current->id != gethighprior(edfqueue)->id)
				{
					// If the current process is finishing at this time then terminate else preempt
					current->rem_time--;
					if(current->rem_time == 0)
					{
						log<<"P"<<current->id<< "finished at time "<< time-1 <<std::endl;
						current->outtime = time-1;
						successfullcomp++;
					}
					else
					{
						log<<"P"<<current->id<<" pre-empted by process "<< "P" << gethighprior(edfqueue)->id <<" at time "<< time << ". Remaining time : " << current->rem_time <<std::endl;
						ispreempted = true;
						current->ispreempted = true;
					}
					break;
				}
				current->rem_time--;
				// If without preemption the current process finishes 
				if(current->rem_time==0)
				{
					log<<"P"<<current->id<<" finished at time "<< time - 1 << std::endl;
					current->outtime = time-1;
					successfullcomp++;
				}
			}	
			if(!ispreempted && !currentmisseddeadline && !currentcompleted)
			{
				int index = getindex(edfqueue,current);

				if(index!=-1)
				{
					edfqueue.erase(index+edfqueue.begin());
				}
			}
		}
		else // Waiting for entry of a new process
		{
			current = NULL;
			bool pushed = false;
			for(unsigned i=0;i<n;i++)
			{
				if(time%processes[i]->period==0)
				{
					// Running until all iterations are completed
					if(processes[i]->round<=processes[i]->k)
					{
						if(processes[i]->reset()!=2)
						{
							pushed = true;
							edfqueue.push_back(processes[i]);
						}
					}
					else
					{
						if(fin[processes[i]->id-1]!=1)
						{
							fin[processes[i]->id-1] = 1;
						}
					}
				}
		}
		if(pushed)
			log<<"CPU is idle till "<<time<<std::endl;
		
		time++;	
		}
	}	
	// Calculating the waiting times and writing to file
	float totalavg = 0.0f;
	stats<<"Number of processes entered : " << tot <<  std::endl;
	stats<<"Number of succesful completions : "<<successfullcomp<<std::endl;
	stats<<"Number of deadlines missed : "<<deadlinemisses<<std::endl;
	for(unsigned i=0;i<processes.size();i++)
	{
		stats<<"P"<<processes[i]->id<<" average wait time : "<<(float)processes[i]->wait_time/processes[i]->k<<std::endl;
		totalavg += processes[i]->wait_time;
	}
	totalavg = totalavg/tot;
	stats<<"Total average waiting time : "<<totalavg<<std::endl;

	// Closing the read,write files
	inp.close();
	log.close();
	stats.close();

	//Freeing the memory allocated to avoid memory leak
	for(unsigned i=0;i<n;i++)
	{
		free(processes[i]);
	}
	return 0;
}