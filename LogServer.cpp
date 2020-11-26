/*
The server creates a socket for UDP communication, there will be receive thread read the message from the logger.
 The server can set the log serverity level by sending a message to the logger. The server can read log message from the server log file and print them to the screen.
*/
#include <errno.h> 
#include <iostream> 
#include <queue> 
#include <signal.h> 
#include <string.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fstream>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
const int BUF_LEN=1024;
bool is_running;
struct sockaddr_in addr;
int portno, fd, log_fd, ret, len;
char buf[BUF_LEN];
pthread_t tid;
pthread_mutex_t lock_x;
void *recv_func(void *arg);

using namespace std;

static void shutdownHandler(int sig)
{
    switch(sig) {
        case SIGINT:
            is_running=false;
	    break;
    }
}

int main(){
    	struct sigaction action;	
	memset(&addr, 0, sizeof(addr));
    	if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 	//create a socket file descriptor
        	cout << "server: " << strerror(errno) << endl;

    	}
 	
	action.sa_handler = shutdownHandler;
    	sigemptyset(&action.sa_mask);
    	action.sa_flags = 0;
    	sigaction(SIGINT, &action, NULL);

//Set the address and port of the server.
	portno = 1200;
    	addr.sin_family=AF_INET;
    	addr.sin_addr.s_addr=INADDR_ANY;
    	addr.sin_port=htons(portno);

	if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {	//bind the socket to the server address
            cout << "Logger: " << strerror(errno) << endl;
            close(fd);
            
        }
 	pthread_mutex_init(&lock_x, NULL);
	ret = pthread_create(&tid, NULL, recv_func, &fd);		//pass file descriptor to receive thread
    	if(ret!=0) {
        	cout<<strerror(errno)<<endl;
    	}

	int choice, level;
	is_running = true;
	while(is_running){

		cout << "1. Set the log level" << endl;
		cout << "2. Dump the log file here" << endl;
		cout << "0. Shut down" << endl;

		cin >> choice;

		switch (choice)
		{
			case 1:
				
				cout << "Please enter the filter log severity:";
				cin >> level;
				memset(buf, 0, BUF_LEN);
				len=sprintf(buf, "Set Log Level=%d", level)+1;
				if(sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
					cout << strerror(errno);
				break;

			case 2:{
				ifstream infile("/tmp/server.log");
            			if(infile.is_open()) {
					string data;
                			while(!infile.eof()) {
						getline(infile,data);
						cout << data << endl;
					}
					infile.close();
					char key;
					cout << "Press any key to continue: ";
					cin >> key;
					system("clear");
				}
			       break;
			       }
			case 0:
				
				is_running = false;	//stop the receive thread and exit the while loop
				break;
				
			default:
				cout << "Please enter 1, 2 or 0" << endl;
				break;

		}
	}
	
	close(log_fd);
	pthread_join(tid, NULL);
	return 0; 
}

void *recv_func(void *arg)
{
	int r_fd = *(int *) arg;
	socklen_t addr_len = sizeof(addr);
	log_fd = open("/tmp/server.log", O_WRONLY | O_CREAT | O_APPEND, 0666);		//open the server log file, if the file does not exit, create a file with rw-rw-rw permission

	while(is_running)
	{	
		pthread_mutex_lock(&lock_x);		//ensure two receives do not run into each other.	
	
		if(recvfrom(r_fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addr_len) < 0)
			cout << "recvfrom: " << strerror(errno) <<endl;

		ret = write(log_fd, buf,sizeof(buf));

		if(ret < 0)
		{
			cout << "write: " << strerror(errno)<<endl;
		}

		pthread_mutex_unlock(&lock_x);
		sleep(1);	//ensure non-blocking


	}

	pthread_exit(NULL);
}
