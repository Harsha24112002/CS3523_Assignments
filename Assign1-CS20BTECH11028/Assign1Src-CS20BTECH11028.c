#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define SEC_TO_MICROSEC 1000000

//Structure for storing coordinates of a point 
struct point{
	int x;
	int y;
};
typedef struct point Point;

//Global variables, as they could be accessed between threads

//Size of the set of points, number of threads,size to be assigned per thread
int n_points,n_threads,sizeperthread;

//Given set of Points
Point *arr;

//Points with minimum distance from the given point found by each thread
Point *min;

//Corresponding minimum distances returned by each thread
int *distance;

//The reference point with respect to which distance is being calculated
Point p;

/**
 * @brief Finds the point with minimum distance from the given point
 * @param id the number of the thread
 * @param start starting point of the subset
 * @param end ending point of the subset
 * */
void find_min_dist(int id,int start,int end)
{
	int min_distance = -1;
	Point min_corr;
	for(unsigned i=start;i<=end;i++)
	{
		int xdist = arr[i].x - p.x;
		int ydist = arr[i].y - p.y;
		/**
		 * As if sqrt((x1-x)^2+(y1-y)^2)>sqrt((x2-x)^2+(y2-y)^2) 
		 * => (x1-x)^2+(y1-y)^2>(x2-x)^2+(y2-y)^2
		 * Hence we can ignore the sqrt so that there won't be any 
		 * floating point accuracy mistakes
		 * */
		int dist = ((xdist*xdist) + (ydist*ydist));
		if(min_distance == -1)
		{
			min_distance = dist;
			min_corr.x = arr[i].x;
			min_corr.y = arr[i].y;
		}
		if(min_distance>dist)
		{
			min_distance = dist;
			min_corr.x = arr[i].x;
			min_corr.y = arr[i].y;
		}
	}
	//Storing the minimum distance and corresponding point in the 
	//global array corresponding to the thread number
	distance[id] = min_distance;
	min[id] = min_corr;

}

/**
 * @brief thread function for passing it to threads
 * */
void* min_dist(void* args)
{
	// i is the thread number used for determining the start and end points of the 
	// subset 
	int i = (int)(long)(args);
	int start = i*sizeperthread;
	int end = (i+1)*sizeperthread-1;

	if(end>n_points-1)
	end = n_points-1;

	if(i==n_threads-1)
	end = n_points-1;

	find_min_dist(i,start,end);
	pthread_exit(0);

}


int main()
{
	FILE* input = fopen("sampleinput.txt","r");
	fscanf(input,"%d",&n_threads);
	char c;
	int count=0;
	// Getting the coordinates of the point
	while(c=fgetc(input))
	{
		if(c==')')break;
		if(c=='(' || c==',')continue;
		if(c>='0' && c<='9')
		{
			if(count==0)
			{
				p.x = c - '0';
				count++;
			}
			else
			{
				p.y = c - '0';
			}
		}
	}
	fscanf(input,"%d",&n_points);
	sizeperthread = n_points/n_threads;

	//Dynamically allocating memories  

	arr = (Point*)malloc(n_points*sizeof(Point));
	min = (Point*)malloc(n_threads*sizeof(Point));
	distance = (int*)malloc(n_threads*sizeof(int));

	for(unsigned i=0;i<n_points;i++)
	{
		char c;
		int count=0;
		while(c=getc(input))
		{
			if(c==')')break;
			if(c=='(' || c==',')continue;
			
			if(c>='0' && c<='9')
			{
				if(count==0)
				{
					arr[i].x = c - '0';
					count++;
				}
				else
				{
					arr[i].y = c - '0';
				}
			}
		}
	}
	//Starting the clock
	double startc = clock();

	pthread_t tid[n_threads];
	pthread_attr_t attr[n_threads];
	//Creating threads
	for(int i=0;i<n_threads;i++)
	{
		pthread_attr_init(&attr[i]);
		pthread_create(&tid[i],&attr[i],min_dist,(void*)(long)(i));
	}

	// Joining threads 
	for(int i=0;i<n_threads;i++)
	{
		pthread_join(tid[i],NULL);
	}
	
	// Finding minimum among the values returned by the thread
	int minimum = distance[0];
	int index;
	for(int i=1;i<n_threads;i++)
	{
		if(minimum>distance[i])
		{
			minimum = distance[i]; 
			index = i;
		}
	}

	//Stopping the clock
	double endc = clock();
	double timetaken = (double)(endc-startc)/CLOCKS_PER_SEC;

	printf("%lf microseconds\n",timetaken*SEC_TO_MICROSEC);
	printf("Point is (%d,%d)\n",min[index].x,min[index].y);
}