
/*https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch*/

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


#define PORTNO     "9000"
#define BACKLOGS   10
#define PERMISSION 0644
#define BUFFER_SIZE  100


int socketfd = 0, socketnewfd = 0,fd=0;

static void signal_handler (int signo)
{
	if(signo == SIGINT)
	{
		syslog(LOG_DEBUG,"Caught Signal SIGINT, exiting");
		//free(receive_buffer);
		//free(send_buffer);
								
	}
	
	if(signo == SIGTERM)
	{
		syslog(LOG_DEBUG,"Caught Signal SIGTERM, exiting");
unlink("/var/tmp/aesdsocketdata");
		//free(receive_buffer);
		//free(send_buffer);
		
		
	}
	exit(EXIT_SUCCESS);
	shutdown(socketfd, SHUT_RDWR);
 	shutdown(socketnewfd, SHUT_RDWR);
 	close(socketfd);
 	close(socketnewfd);
 	unlink("/var/tmp/aesdsocketdata");
 	//free(receive_buffer);
	exit(EXIT_SUCCESS);				/*TO DO: Graceful exit what we need to return?*/
}


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

void daemon_mode()
{
	 	/* create new process */
 	pid_t pid = fork ( );
 	if (pid == -1)
 	exit(-1);
 		
 	else if (pid != 0)
 		exit (EXIT_SUCCESS);
 	/* create new session and process group */
	 if (setsid ( ) == -1)
 		exit(-1);
 	/* set the working directory to the root directory */
 	if (chdir ("/") == -1)
 		exit(-1);

 	/* redirect fd's 0,1,2 to /dev/null */
 	
 	
 	open ("/dev/null", O_RDWR); /* stdin */
 	dup (0); /* stdout */
 	dup (0); /* stderror */
}

int main(int argc,  char *argv[])
{


	struct addrinfo hints;
	struct addrinfo *result;  // will point to the results
	struct sockaddr host_address;
	socklen_t host_address_size;
	int receive_bytes = 0;
	char temp_buffer[BUFFER_SIZE];
	 /*TO DO: Check is this line is reuqired*/
	int byte_count=0;
	int total_packet_size=0;
	char *send_buffer = NULL;
	int bytes_read=0;
	int status=0;

	
		
	//signal_init();
	openlog("AESDSocket",LOG_PID,LOG_USER);
	printf("Welcome to Socket \n");
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
	status =  getaddrinfo(NULL, PORTNO , &hints, &result);
        if (status != 0)
        {
      		syslog(LOG_ERR,"Getaddrinfo error");  	
      		exit(-1);
        }
        socketfd= socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (socketfd == -1)
        {
        	syslog(LOG_ERR,"Socket Creation error");  	
      		exit(-1);
        
        }
	status= bind(socketfd, result->ai_addr, result->ai_addrlen);
	if (status == -1)
        {
        	syslog(LOG_ERR,"Bind error");  	
      		exit(-1);
        
        }
        
         freeaddrinfo(result);
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
        
        fd = creat("/var/tmp/aesdsocketdata", 0644);
        if (fd == -1)
        {
        
        	syslog(LOG_ERR,"File could not be created");
        	exit(-1);        	
        }
      

        while (1)
        {
        	char *receive_buffer = (char *) malloc (BUFFER_SIZE * sizeof(char));
		if (receive_buffer == NULL)
		{
			syslog(LOG_ERR,"malloc() error"); 
			printf("Malloc Error \n");
      			exit(-1);
	
		}
		memset(receive_buffer,0,BUFFER_SIZE);
        	bool packet_complete = false;
                memset(temp_buffer, 0, BUFFER_SIZE);
        	host_address_size = sizeof(struct sockaddr);
        	socketnewfd= accept(socketfd, (struct sockaddr *)&host_address, &host_address_size);
        	if (socketnewfd == -1)
        	{
        		syslog(LOG_ERR,"Accept comamnd error");
        		exit(-1);
        	}
        	syslog(LOG_DEBUG, "Accepted connection from XXX\n"); /*TODO: check XXX*/
        	printf("Accepted connection from XXX\n");
        	
        	while (packet_complete == false)
        	{
        		receive_bytes = recv(socketnewfd, temp_buffer, BUFFER_SIZE, 0);
        		if (receive_bytes == -1)
        		{
        			syslog(LOG_ERR,"Accept comamnd error");
        			printf("Receive comamnd error\n");
        			exit(-1);
        		}
			else if (receive_bytes == 0)
			{
				syslog(LOG_DEBUG,"Connection Closed");
        			printf("Connection closed r\n");
        			break;
			}
			for (byte_count = 0; byte_count < BUFFER_SIZE; byte_count++)
			{
				if(temp_buffer[byte_count] == '\n')
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
			
		        fd = open("/var/tmp/aesdsocketdata", O_RDWR | O_APPEND);
			if (fd == -1)
			{
				syslog(LOG_ERR,"File cannot be opened in WRITE mode");
				perror("WRITE");
				exit(-1);
			}
			lseek(fd, 0, SEEK_END);
			status = write(fd, receive_buffer, strlen (receive_buffer));
			if (status == -1)
			{
				syslog(LOG_ERR,"The received data is not written ");
				exit(-1);
			}
			lseek(fd, 0, SEEK_SET);
			send_buffer = (char *) malloc(total_packet_size * sizeof(char));
		        bytes_read = read(fd, send_buffer, total_packet_size );
			status = send(socketnewfd, send_buffer, bytes_read, 0);
			if(status == -1)
			{
				syslog(LOG_ERR,"Sending to host failed");
				exit(-1);
			}
			free(receive_buffer);
			free(send_buffer);
		   	close(fd);	        
		   	syslog(LOG_DEBUG, "Closed connection from XXX\n"); /*TODO: check XXX*/
        	        printf("Closed connection from XXX\n");
		       
        	 
        
        
	}
	
		  	
	

}
