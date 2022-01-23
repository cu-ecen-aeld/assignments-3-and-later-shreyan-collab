
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>

char *errorMessageArgument = "ERROR: Invalid Number of arguments\nTotal number of arguments must be 2.\nThe order of arguments should be\n1.File path \n2.String to be written to the path\n";

int main(int argc, char *argv[])
{
	openlog("Writer-Logging messages",0,LOG_USER); /*Opens connection to the system logger */
	char *fullpath = argv[1]; /*Parsing path from command line argument*/
	char *inputstring = argv[2]; /*Parsing input string from command line argument*/
	int status; /*To store the status for file functions*/
	if(argc != 3)
	{
		printf("%s",errorMessageArgument);
		syslog(LOG_ERR,"ERROR:Invalid No of Arguments: %d\n",argc);
		return 1;
	}
	status = creat (fullpath, 0644);
	if (status == -1)
	{
		printf("ERROR: Cannot create the file in the path:%s\n",fullpath);
		syslog(LOG_ERR,"ERROR: Cannot create the file in the path:%s\n",fullpath);
	}
	else
	{
		printf("LOG: File is created in the path:%s\n",fullpath);
		syslog(LOG_DEBUG,"LOG: File is created in the path: %s\n",fullpath);
	}
	
	status = open (fullpath, O_WRONLY);/*Using write only flag*/
	if(status == -1)
	{
		printf("ERROR: Cannot open the file located at:  %s\n",fullpath);
		syslog(LOG_ERR,"ERROR: Cannot open the file located at:  %s\n",fullpath);
	}
	else
	{	printf("LOG: Opened the file located at: %s\n",fullpath);
		syslog(LOG_DEBUG,"LOG: Opened the file located at: %s\n",fullpath);
	}
	status = write (status, inputstring,strlen(inputstring)); 	     		if(status == -1)
	{
		printf("ERROR: Cannot write %s to the file located at:%s\n",inputstring,fullpath);
		syslog(LOG_ERR,"ERROR: Cannot write %s to the file located at:%s\n",inputstring,fullpath);
	}
	else
	{
		printf("LOG: Written %s to the file located at: %s\n",inputstring,fullpath);
		syslog(LOG_DEBUG,"LOG: Written %s to the file located at:%s\n",inputstring,fullpath);
	
	}
	close(status);
	return 0;
	
}


