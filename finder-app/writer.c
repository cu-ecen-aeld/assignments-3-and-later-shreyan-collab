/*
@File name :writer.c
@brief     :To create a file in specified directory and search a word in the file if it exists or not
@author    :Shreyan Prabhu 
@Date	   :01/23/2022
@References:https://linux.die.net/man/2/access
*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>

#define TOTAL_ARGUMENTS 3
#define STATUS_ERROR   -1
#define SUCCESS 	 0
#define FAILURE         1
#define PERMISSION      0644 /*Giving user read, write permission, group - read and others read permission only*/

char *errorMessageArgument = "ERROR: Invalid Number of arguments\nTotal number of arguments must be 2.\nThe order of arguments should be\n1.File path \n2.String to be written to the path\n";

int main(int argc, char *argv[])
{
	openlog("Writer-Logging messages",0,LOG_USER); /*Opens connection to the system logger */
	char *fullpath = argv[1]; /*Parsing path from command line argument*/
	char *inputstring = argv[2]; /*Parsing input string from command line argument*/
	int fd; /*File descriptor To store status for file functions*/
	if(argc != TOTAL_ARGUMENTS)
	{
		printf("%s",errorMessageArgument);
		syslog(LOG_ERR,"ERROR:Invalid No of Arguments: %d\n",argc);
		return FAILURE;
	}
	if (access(fullpath,F_OK) == STATUS_ERROR) /*Checking whether file exists*/
	{
		syslog(LOG_DEBUG,"DEBUG:File is not present, creating a file now\n"); 
		fd = creat (fullpath, PERMISSION); 
	}
	if (fd == STATUS_ERROR)
	{
		printf("ERROR: Cannot create the file in the path:%s\n",fullpath);
		syslog(LOG_ERR,"ERROR: Cannot create the file in the path:%s\n",fullpath);
		close(fd);
		return FAILURE;
		
	}
	else
	{
		printf("DEBUG: File is created in the path:%s\n",fullpath);
		syslog(LOG_DEBUG,"DEBUG: File is created in the path: %s\n",fullpath);
	}
	
	fd = open (fullpath, O_WRONLY);/*Using write only flag*/
	if(fd == STATUS_ERROR)
	{
		printf("ERROR: Cannot open the file located at:  %s\n",fullpath);
		syslog(LOG_ERR,"ERROR: Cannot open the file located at:  %s\n",fullpath);
		close(fd);
		return FAILURE;
	}
	else
	{	printf("DEBUG: Opened the file located at: %s\n",fullpath);
		syslog(LOG_DEBUG,"DEBUG: Opened the file located at: %s\n",fullpath);
	}
	fd = write (fd, inputstring,strlen(inputstring)); 	     		if(fd == STATUS_ERROR)
	{
		printf("ERROR: Cannot write %s to the file located at:%s\n",inputstring,fullpath);
		syslog(LOG_ERR,"ERROR: Cannot write %s to the file located at:%s\n",inputstring,fullpath);
		close(fd);
		return FAILURE;
	}
	else if ( fd != strlen(inputstring))
	{
		printf("ERROR: The string %s is partially written to the file located at %s\n",inputstring,fullpath);
		syslog(LOG_ERR,"ERROR: The string %s is partially written to the file located at %s\n",inputstring,fullpath);
		close(fd);
		return FAILURE;
	}
	else
	{
		printf("DEBUG: Written %s to the file located at: %s\n",inputstring,fullpath);
		syslog(LOG_DEBUG,"DEBUG: Written %s to the file located at:%s\n",inputstring,fullpath);
	
	}
	close(fd);
	return SUCCESS;
	
}


