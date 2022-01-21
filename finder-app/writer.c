
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
char * errorMessage = "ERROR: Invalid Number of arguments\nTotal number of arguments must be 2.\nThe order of arguments should be\n1.File path \n2.String to be written to the path";



int main(int argc, char *argv[])
{
	char *fullpath = argv[1];
	char *inputstring = argv[2];
	ssize_t statusfilewrite;
	if(argc != 3)
	{
		return 1;
		//printf("Error message"); USE SYSLOG
	}
	int statuscreate = creat (fullpath, 0644);
	//int fd = open(fullpath, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP 		| S_IROTH);
	if (statuscreate == -1)
	{
		//Error in file creating
	}
	int statusfileopen = open (fullpath, O_WRONLY);
	if(statusfileopen == -1)
	{
		//TO DO : Error in file creating
	}
	statusfilewrite = write (statusfileopen, inputstring, 	                  strlen(inputstring));
	if(statusfilewrite == -1)
	{
		// TO DO: Error in file writing
	}
	/*if(fputs(inputstring,stream) == EOF )
	{
		// ERROR HANDLING
	}*/
	close(statusfileopen);
	/*
	if(close(stream) == EOF)
	{
		// ERROR HANDLING
	}
	*/
	
}


