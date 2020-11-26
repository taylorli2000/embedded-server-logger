/*InitializeLog() creates a non-blocking socket for udp communication and creates a thread mutex for the receive thread
 *SetLogLevel(LOG_LEVEL) set the filter log level severity 
 *Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message) create a log message and send it to the server using UDP 
 *ExitLog() close file descriptor and join thread to the main thread
 * */

#include"Logger.h"

LOG_LEVEL lvl = DEBUG;
void *recv_func(void *arg);
bool is_running;
int BUF_LEN=1024;
struct sockaddr_in addr;
int portno, fd, ret;
pthread_t tid;
pthread_mutex_t lock_x; 
int  InitializeLog(){

//create a non-blocking socket for UDP communications (AF_INET, SOCK_DGRAM).
	memset(&addr, 0, sizeof(addr));
    	if ( (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        	cout << "server: " << strerror(errno) << endl;
        	return -1;
    	}

//Set the address and port of the server
	portno = 1200;
	addr.sin_family = AF_INET;
    	addr.sin_addr.s_addr=INADDR_ANY;
    	addr.sin_port=htons(portno);

	//Initialize a thread mutex.
//Start the receive thread and pass the file descriptor to it.
	pthread_mutex_init(&lock_x, NULL);
	is_running = true;
	ret = pthread_create(&tid, NULL, recv_func, &fd);
    	if(ret!=0) {
        	cout<<strerror(errno)<<endl;
    	}
	return 0;

}

void SetLogLevel(LOG_LEVEL level){
//set the filter log level and store in a variable global within Logger.cpp.
	lvl = level;
}

void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message){
//Compare the severity of the log to the filter log severity. The log will be thrown away if its severity is lower than the filter log severity.
	char buf[BUF_LEN];
	int len;
	if(level >= lvl)
	{
		time_t now = time(0);
        	char *dt = ctime(&now);
        	memset(buf, 0, BUF_LEN);
        	char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
        	len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog, func, line, message)+1;
        	buf[len-1]='\0';
//The message will be sent to the server via UDP sendto().
		if (sendto(fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			cout << strerror(errno) << endl;
		}
	}
}

void ExitLog(){
//stop the receive thread via an is_running flag and close the file descriptor.
	is_running = false;
	close(fd);
	pthread_join(tid, NULL);
}

void *recv_func(void *arg)
{
	int r_fd = *(int *) arg;
	socklen_t addr_len = sizeof(addr);
    	char buf[BUF_LEN];
	while(is_running) {
		pthread_mutex_lock(&lock_x); 	//ensure two receives do not run into each other.
		if(recvfrom(r_fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addr_len) < 0)
			cout << strerror(errno) <<endl;

		if(strncmp(buf,"Set Log Level=",14) == 0){
			char a = buf[14];	
			int templvl = a - '0';	//cast the char to an int type
		 	lvl = static_cast<LOG_LEVEL> (templvl);
		}
		pthread_mutex_unlock(&lock_x);
		sleep(1);	//ensure non-blocking
    	}
    	pthread_exit(NULL);
}

