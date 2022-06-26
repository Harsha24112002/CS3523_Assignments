* Unzip the given zip file, and cd into the directory containing source files
* Place a file named "inp-params.txt" if there is no such file, in the following manner:
		<Num_Readers> <Num_Writers> <kr> <kw> <mucs> <murem>
	Example input:
		10 10 10 10 20 30 

* For compiling 
	* Reader Writer
		g++ Assn5-rw-CS20BTECH11028.cpp -o rw -lpthread -lrt
	* Fair Reader Writer 
		g++ Assn5-frw-CS20BTECH11028.cpp -o frw -lpthread -lrt

* For running
	* Reader Writer
		./rw
	* Fair Reader Writer
		./frw

* The log files will be produced after running in the same directory, RW-log.txt, FRW-log.txt
* The average waiting times will be present in the file Average_time.txt

Note: The Average_time.txt is appended everytime, so if need fresh delete the existing Average_time.txt and run