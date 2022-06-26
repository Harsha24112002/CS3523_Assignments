After downloading the zip file,unzip it
In the folder where the source code is present, place a file named 'inp-params.txt', (if not already present)

Give the input as 
number of processes 'n' ,in the next n lines give description about each process
process id 'pid' processing time(t) period(t) number of iterations(k)
	
		For example : 
		2
		1 25 50 4
		2 35 80 2

Open the terminal and cd into the folder where the source code is located,
For compiling Rate Monotonic Scheduling Simulation

	g++ Assgn3-RMSCS20BTECH11028.cpp -o RMS

For compiling Earliest Deadline First Scheduling Simulation

	g++ Assgn3-EDFCS20BTECH11028.cpp -o EDF

For running Rate Monotonic Scheduling Simulation
	./RMS

For running Earliest Deadline First Scheduling Simulation
	./EDF
	
You would get the output :
	for RMS in : RMS-log.txt and RMS-stats.txt
	for EDFS in : EDF-log.txt and EDF-stats.txt
