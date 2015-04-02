###MikMak

This is an assignment for my Intro to Operating Systems (CPS590) course and is an implementation of the social networking app "YikYak" in C made to run on Ryerson University's Moon Servers with a text-based user interface. There are two executables: MikMak, the client side executable, and MikMakServer, the server side executable. The MikMak client utilizes POSIX Message Queues to exploit interprocess communication to communicate with MikMakServer along with pthreads for the message queues to work on. 

MikMak can be run once MikMakServer is running. An ideal setup would be to run MikMakServer, then in another shell, run MikMak. There are no command line arguments for either. The syntax for running them is as follows:

./MikMakServer

./MikMak