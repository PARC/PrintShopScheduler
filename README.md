# PrintShopScheduler
Print Shop Scheduler

Installation (tested on Ubuntu 14.04)
------------

1. Install Boost C++ libraries
 ``` bash
$ sudo apt-get install libboost-all-dev
 ```

2. Install JAVA 8 (optional)
 ``` bash
$ sudo add-apt-repository ppa:webupd8team/java
$ sudo apt-get update
$ sudo apt-get install oracle-java8-installer
 ```

3. Compile the source code of the scheduler. In the root directory of the project:
 ``` bash
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
 ```
All executables produced by the 'make' command above are located in build/apps, including single-site, multi-site, and filler-scheduler. There is a 'run.sh' script in each of the folders under build/apps. The names of the executables are: psss (single-site scheduler), pssm (multi-site scheduler), and pssf (filler scheduler), which can be used to test the excess capacity of a given shop using a test filler job. Running the executables without any arugment will print out the usage message to stdout. Directory build/apps/jni contains a JNI interface for the scheduler.

Example shop and job files
---------------------

The 'data' folder contains a list of example shops and job lists, as follows:

1. example.shp: a single-site shop example

2. example_joblist[1-5].jls: example job list files

3. example_multi_site_shop.mss: a multi-site shop example that refers to the following three site shops

4. example_shop[1-3].shp: example single-site shops that belong to the above multi-site shop

5. example_multi_site_jobs.msj: a multi-siite job list example that refers to the five single-site job lists described in 2.
 
