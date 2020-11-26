/*InitializeLog() creates a non-blocking socket for udp communication and creates a thread mutex for the receive thread
 *SetLogLevel(LOG_LEVEL) set the filter log level severity 
 *Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message) create a log message and send it to the server using UDP 
 *ExitLog() close file descriptor and join thread to the main thread
 * */
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
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

using namespace std;
enum LOG_LEVEL
{
DEBUG, 
WARNING, 
ERROR, 
CRITICAL
};

int InitializeLog();
void SetLogLevel(LOG_LEVEL level);
void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message);
void ExitLog();

