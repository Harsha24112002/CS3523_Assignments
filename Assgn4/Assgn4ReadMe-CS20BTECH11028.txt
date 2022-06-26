* Unzip the file and cd into the directory containing source files
* To give the input place a file named "inp-params.txt" if it doesn't exist already.
* In the file, enter number of threads, number of calls to CS, lambda1, lambda2. 
		For example : 10 10 3 5
	
* To compile the files, in the same directory 
	*For TAS 
		g++ SrcAssgn4-tas-CS20BTECH11028.cpp -o tas -lpthread
	*For CAS
		g++ SrcAssgn4-cas-CS20BTECH11028.cpp -o cas -lpthread
	*For Bounded Waiting CAS
		g++ SrcAssgn4-casbounded-CS20BTECH11028.cpp -o boundedwaitcas -lpthread

* To run the files, in the same directory
	*For TAS 
		./tas
	*For CAS 
		./cas
	*For Bounded wait cas
		./boundedwaitcas

* You will get the corresponding outputs in TAS-Log.txt, CAS-Log.txt, BoundedCas-log.txt files.
