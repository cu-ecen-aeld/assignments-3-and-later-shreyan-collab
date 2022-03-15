/*
*@Name: aesdsocket.c
*@Author: Shreyan Prabhu D
*@Brief: Source file to start aesd application which can send and receive pockets
*@Reference:https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch
*		  : Linux System Programming Robert Love
*/

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define PORTNO     "9000"
#define BACKLOGS    10
#define PERMISSION  0644
#define BUFFER_SIZE 100

#define USE_AESD_CHAR_DEVICE 1

#ifdef USE_AESD_CHAR_DEVICE
#define PATH "/dev/aesdchar"
#else
#define PATH "/var/tmp/aesdsocketdata"
#endif

int socketfd = 0, socketnewfd = 0,fd=0;
char *receive_buffer = NULL;
int total_packet_size=0;
SLIST_HEAD(slisthead, slist_data_s) head;
typedef struct thread_data
{
	pthread_mutex_t *mutex;
	pthread_t thread_id;
	bool thread_complete_success;
	int accept_socket_fd;
}thread_data_t;

struct slist_data_s
{
	thread_data_t thread_data_args;
	SLIST_ENTRY(slist_data_s) entries;
};
typedef struct slist_data_s slist_data_t;
pthread_mutex_t mutex1;

/*
*@Function: signal_handler 
*@brief: Handler function for handling different signals
*@argument: Signal Type
*@Return: None
*/
static void signal_handler (int signo)
{
	if(signo == SIGINT)
	{
		syslog(LOG_DEBUG,"Caught Signal SIGINT, exiting");						
	}
	
	if(signo == SIGTERM)
	{
		syslog(LOG_DEBUG,"Caught Signal SIGTERM, exiting");
		
	}

	shutdown(socketfd, SHUT_RDWR);
 	shutdown(socketnewfd, SHUT_RDWR);
 	close(socketfd);
 	close(socketnewfd);
 	close(fd);
 	unlink(PATH);
 	slist_data_t *add_thread = NULL;
 	SLIST_FOREACH(add_thread, &head, entries)
        {
		pthread_join(add_thread->thread_data_args.thread_id, NULL);
        	SLIST_REMOVE(&head, add_thread, slist_data_s, entries);
        	free(add_thread);
        	break;
      	}
	exit(0);				
}

/*
*@Function: signal_init 
*@brief: Initializing the signals
*@argument: None
*@Return: None
*/
void signal_init()
{
	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		syslog(LOG_ERR,"Error in handling SIGINT");
		exit(-1);
	}
	if (signal(SIGTERM, signal_handler) == SIG_ERR)
	{
		syslog(LOG_ERR,"Error in handling SIGINT");
		exit(-1);
	}


}

/*
*@Function: daemon_mode
*@brief: To start a process in daemon mode
*@argument: None
*@Return: None
*/
void daemon_mode()
{
 	pid_t pid = fork ( );				/*Through  fork command creating new process*/
 	if (pid == -1)
 	exit(-1);
 		
 	else if (pid != 0)				/* Creating process group and new session */
 		exit (0);
 		
	 if (setsid ( ) == -1)				/*set the working directory to the root directory */
 		exit(-1);
 	
 	if (chdir ("/") == -1)
 		exit(-1);

 	open ("/dev/null", O_RDWR); 			/* redirect fd's 0,1,2 to /dev/null */ /* stdin */
 	dup (0); 					/* stdout */
 	dup (0); 					/* stderror */
}
 
 #ifndef USE_AESD_CHAR_DEVICE
/*
*@Function: alarm_handler
*@brief: Consists of handling function when an alarm is triggered
*@argument: Alarm type
*@Return: None
*/
void alarm_handler (int alarm)
{

	
	int status =0;
	int fd=0;
	char time_string[100];
	time_t rawtime;
	struct tm* currenttime;
	time (&rawtime);
	currenttime = localtime (&rawtime);
	strftime(time_string, sizeof(time_string), "timestamp:%T %d %b %a %Y %z\n", currenttime); /*RFC 2822 complaint strftime format*/
	
	fd = open(PATH, O_RDWR | O_APPEND , 0644);
	if (fd == -1)
	{
		syslog(LOG_ERR,"File cannot be opened in WRITE mode");
		perror("WRITE");
		exit(-1);
	}
	syslog(LOG_DEBUG, "OPEN status %d", fd);
	status = pthread_mutex_lock(&mutex1);
	if(status)
	{
		syslog(LOG_ERR, "Mutex_lock error");
		
	}
	
	lseek(fd, 0, SEEK_END);
	status = write(fd, time_string, strlen (time_string));
	syslog(LOG_DEBUG, "Number of bytes written %d", status);
	if (status == -1)
	{
		syslog(LOG_ERR,"The received data is not written");
		exit(-1);
	}
	status = pthread_mutex_unlock(&mutex1);
	if(status) 
	{
		syslog(LOG_ERR, "ERROR: mutex_lock() fail");
		
	}
	close(fd);
	
	
}
#endif
/*
*@Function: thread_func
*@brief: Starting function of the new thread
*@argument: The thread parameters
*@Return: void pointer 
*/
void *thread_func(void *thread_param)
{

	char *send_buffer = NULL;
	int bytes_read=0;
	int byte_count=0;
	int receive_bytes = 0;
	int status=0;
  	bool packet_complete = false;
  	
  	char temp_buffer[BUFFER_SIZE];
	receive_buffer = (char *) malloc (BUFFER_SIZE * sizeof(char));
	thread_data_t *thread_data = (thread_data_t *)thread_param;
	if (receive_buffer == NULL)
	{
		syslog(LOG_ERR,"malloc() error"); 
		printf("Malloc Error \n");
      		exit(-1);
	
	}
	memset(receive_buffer,0,BUFFER_SIZE);
 
        memset(temp_buffer, 0, BUFFER_SIZE);
        
        while (packet_complete == false)
        	{
        		receive_bytes = recv(thread_data->accept_socket_fd, temp_buffer, BUFFER_SIZE, 0);
        		if (receive_bytes == -1)
        		{
        			syslog(LOG_ERR,"Accept command error");
        			printf("Receive comamnd error\n");
        			exit(-1);
        		}
			else if (receive_bytes == 0)				/*No data is received*/
			{
				syslog(LOG_DEBUG,"Connection Closed");
        			printf("Connection closed r\n");
        			break;
			}
			for (byte_count = 0; byte_count < BUFFER_SIZE; byte_count++)
			{
				if(temp_buffer[byte_count] == '\n')			/*Checking if new packet is received*/
				{
					packet_complete = true;
					syslog(LOG_DEBUG,"String received is %s",temp_buffer);
					break;
				}
			
			}
		        
		        total_packet_size = total_packet_size + receive_bytes;
		        receive_buffer = (char *) realloc (receive_buffer, total_packet_size + 1);
		        if (receive_buffer == NULL)
		        {
		        	syslog(LOG_ERR,"Realloc() Error");
        			printf("Realloc() Error \n");
        			break;
		        } 
		        strncat(receive_buffer, temp_buffer, receive_bytes);
			syslog(LOG_DEBUG,"Appended String is %s",receive_buffer);
			}
			syslog(LOG_DEBUG,"Appended String is %s",receive_buffer);
		        fd = open(PATH, O_RDWR | O_APPEND , 0644);
			if (fd == -1)
			{
				syslog(LOG_ERR,"File cannot be opened in WRITE mode");
				perror("WRITE");
				exit(-1);
			}
			syslog(LOG_DEBUG, "OPEN status %d", fd);
			status = pthread_mutex_lock(thread_data->mutex);
			if(status)
			{
				syslog(LOG_ERR, "ERROR: mutex_lock() fail");
		
			}
	
			lseek(fd, 0, SEEK_END);
			status = write(fd, receive_buffer, strlen (receive_buffer));
			if (status == -1)
			{
				syslog(LOG_ERR,"The received data is not written ");
				exit(-1);
			}
			status = pthread_mutex_unlock(thread_data->mutex);
			if(status) 
			{
			syslog(LOG_ERR, "ERROR: mutex_lock() fail");
			}
			int file_size = lseek(fd, 0, SEEK_CUR);
			lseek(fd, 0, SEEK_SET);
			send_buffer = (char *) malloc(file_size * sizeof(char));
			status = pthread_mutex_lock(thread_data->mutex);
		        bytes_read = read(fd, send_buffer, file_size );
			if(bytes_read == -1)
			{
			
				syslog(LOG_ERR,"Error in Readingk");
				//exit(-1);
			}
			status = send(thread_data->accept_socket_fd, send_buffer, bytes_read, 0);
			if(status == -1)
			{
				syslog(LOG_ERR,"Sending to host failed");
				//exit(-1);
			}
			status = pthread_mutex_unlock(thread_data->mutex);
			thread_data->thread_complete_success=true;
			close(fd);
			close(thread_data->accept_socket_fd);
			free(send_buffer);					/*Freeing all the buffers*/
			free(receive_buffer);	     
			receive_buffer=NULL;
			send_buffer=NULL;   
			return (thread_data);
		  
 }
		       
/*
*@Function: main
*@brief: consists of Socket initialization, creating new threads and thread cleanup functions
*@argument: argc- number of command line arguments,  argv -command line arguments stored in an array
*@Return: 0 for success and 1 for failure
*/
int main(int argc,  char *argv[])
{
	
	
	struct addrinfo hints;
	struct addrinfo *result, *temp_ptr;  	
	struct sockaddr_in host_address;
	socklen_t host_address_size;
	slist_data_t *add_thread=NULL;
	int status=0;
	
	SLIST_INIT(&head);
	
	signal_init();
	openlog("AESDSocket",LOG_PID,LOG_USER);
	printf("Welcome to Socket \n");
	memset(&hints, 0, sizeof hints); /* Ensuring the structure is empty*/
	hints.ai_family = AF_UNSPEC;     /* Not specifying IPV4 or IPV6*/
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_flags = AI_PASSIVE;    
	status =  getaddrinfo(NULL, PORTNO , &hints, &result);
        if (status != 0)
        {
      		syslog(LOG_ERR,"Getaddrinfo error");  	
      		exit(-1);
        }
        
        /*Reusing the socket*/
        for(temp_ptr = result; temp_ptr != NULL; temp_ptr = temp_ptr->ai_next)
        { 
        	socketfd= socket(result->ai_family, result->ai_socktype, 0);
        	if (socketfd == -1)
        	{
        		syslog(LOG_ERR,"Socket Creation error");  	
      			exit(-1);
        
       	}
       	status = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
		status= bind(socketfd, result->ai_addr, result->ai_addrlen);
		if (status == 0)
        	{
        		syslog(LOG_DEBUG,"Bind successfull");  	
      			break; 
       	}
       }
        
       freeaddrinfo(result);
       if(temp_ptr == NULL)
       {
       	syslog(LOG_DEBUG,"ERROR: bind() fail");
       	exit(-1);
       
       }
	if( argc > 1 && !strcmp("-d", argv[1]) ) 
	{
		syslog(LOG_INFO, "Running as daemon");
		daemon_mode();
	}
	status= listen(socketfd, BACKLOGS);
       if (status == -1)
       {
        	syslog(LOG_ERR,"Listen error");  	
      		exit(-1);
        
        }
        
       	fd = creat(PATH, 0644);
        	if (fd == -1)
        	{
        		syslog(LOG_ERR,"File could not be created");
        		exit(-1);        	
        	}
      #ifndef USE_AESD_CHAR_DEVICE
        struct itimerval delay;
	signal (SIGALRM, alarm_handler);
	delay.it_value.tv_sec =10;
	delay.it_interval.tv_sec=10;
	delay.it_value.tv_usec =0;
	delay.it_interval.tv_usec=0;
      	status = setitimer (ITIMER_REAL, &delay, NULL);
	if(status)
	{
		syslog(LOG_ERR, "Error setting timer");
		exit (-1);
	}
	#endif
	status = pthread_mutex_init(&mutex1, NULL);
	if (status != 0)
	{
		syslog(LOG_ERR, "Mutex Initialization error");
		exit (-1);
	
	}
        while (1)
        {

        	host_address_size = sizeof(struct sockaddr);
        	socketnewfd= accept(socketfd, (struct sockaddr *)&host_address, &host_address_size);
        	if (socketnewfd == -1)
        	{
        		syslog(LOG_ERR,"Accept comamnd error");
        		exit(-1);
        	}
        	syslog(LOG_DEBUG, "Accepted connection from %s\n", inet_ntoa(host_address.sin_addr)); 
        	printf("Accepted connection from %s\n", inet_ntoa(host_address.sin_addr));
        	add_thread = (slist_data_t *) malloc(sizeof(slist_data_t));  
        	SLIST_INSERT_HEAD(&head, add_thread, entries);     
        	add_thread->thread_data_args.mutex = &mutex1;
        	add_thread->thread_data_args.accept_socket_fd = socketnewfd;
        	add_thread->thread_data_args.thread_complete_success = false;
        	pthread_create(&(add_thread->thread_data_args.thread_id), NULL, thread_func, &(add_thread->thread_data_args));
        	
        	/*Removing nodes by traversing through the linked list*/
        	SLIST_FOREACH(add_thread, &head, entries)
        	{
        		pthread_join(add_thread->thread_data_args.thread_id, NULL);
        		SLIST_REMOVE(&head, add_thread, slist_data_s, entries);
        		free(add_thread);
        		syslog(LOG_DEBUG, "Closed connection from %s\n",inet_ntoa(host_address.sin_addr)); 
        	      	printf("Closed connection from %s\n",inet_ntoa(host_address.sin_addr));
        	      	break;
        	        	
        	}
        	
        	
	}
	
	

}

