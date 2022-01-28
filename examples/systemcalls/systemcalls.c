#include "systemcalls.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <syslog.h>

#define ERROR_STATUS -1
#define SUCCESS_STATUS 0
#define PERMISSION 644				
/**
 * @param cmd the command to execute with system()
 * @return true if the commands in ... with arguments @param arguments were executed 
 *   successfully using the system() call, false if an error occurred, 
 *   either in invocation of the system() command, or if a non-zero return 
 *   value was returned by the command issued in @param.
*/
bool do_system(const char *cmd)
{

    openlog("Systemcalls.c-Log Messages for system() call", 0 , LOG_USER);
    int status = 0;
    status = system(cmd);        	/*Making a system call*/
    if ( status == ERROR_STATUS)
    {
  	syslog(LOG_ERR,"ERROR: System Call failed");
  	printf("ERROR: System Call failed\n");
  	closelog();
    	return false;
    }
    else
    {
    	syslog(LOG_DEBUG,"DEBUG: System call is executed");
    	closelog();
    	return true;
    }
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the 
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

    pid_t pid;
    pid = fork();
    if (pid == ERROR_STATUS)
    {
    	syslog(LOG_ERR,"ERROR: Executing fork command");
    	printf("ERROR: Executing fork command\n");
    	return false;
    }
    else if (pid == SUCCESS_STATUS) /*In child, fork() successfull return 0*/
    {

    	int execv_status = 0;
    	execv_status = execv(command[0], command);
    	if(execv_status == ERROR_STATUS)
    	{
    		syslog(LOG_ERR,"ERROR: Exeuting execv command");
    		printf("ERROR: Exeuting execv command\n");
    		exit(ERROR_STATUS);
    	}
      	
    }
    else if (pid > 0) /*In parent,fork() returns child pid,greater than 0*/
    {
    	int wait_status = 0;
    	pid_t pid1 = waitpid (pid, &wait_status, 0);
    	if(pid1 == ERROR_STATUS)
    	{
    		syslog(LOG_ERR,"ERROR: Executing wait  command");
    		printf("ERROR: Executing wait  command\n");
    		return false;
    	}
    	if (WIFEXITED (wait_status) == 0 || WEXITSTATUS (wait_status))
    	{
    		syslog(LOG_ERR,"ERROR: Child process didn't terminate properly");
    		printf("ERROR: Child process didn't terminate properly\n");
    		return false;
    	}
    
    } 
    va_end(args);
    syslog(LOG_DEBUG,"DEBUG: Normal termination of child process");
    closelog();	
    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.  
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *   
*/
    openlog("Systemcalls.c-Log Messages for execv_redirect function", 0 , LOG_USER);
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, PERMISSION);
    if (fd == ERROR_STATUS) 
    { 
    	syslog(LOG_ERR,"ERROR: Cannot open the file");
    	printf("ERROR: Cannot open the file\n");
    	return false;
    }
    pid_t pid;
    pid = fork();
    if (pid == ERROR_STATUS)
    {
    	syslog(LOG_ERR,"ERROR: Executing fork command");
    	printf("ERROR: Executing fork command\n");
    	return false;
    }
    else if (pid == SUCCESS_STATUS) /*In child, fork() successfull return 0*/
    {
    	if (dup2(fd, 1) < 0)   /*Redirecting standard output to a file specified by output file */
    	{ 
    		syslog(LOG_ERR,"ERROR: Could not redirect standard output to output file");
    		printf("ERROR: Could not redirect standard output to output file\n");
    		return false;
    	}
   	close(fd);
    	int execv_status = 0;
    	execv_status = execv(command[0], command);
    	if(execv_status == ERROR_STATUS)
    	{
    		syslog(LOG_ERR,"ERROR: Exeuting execv command");
    		printf("ERROR: Exeuting execv command\n");
    		exit(ERROR_STATUS);
    	}
      	
    }
    else if (pid > 0) /*In parent,fork() returns child pid,greater than 0*/
    {
    	int wait_status = 0;
    	pid_t pid1 = waitpid (pid, &wait_status, 0);
    	if(pid1 == ERROR_STATUS)
    	{
    		syslog(LOG_ERR,"ERROR: Executing wait  command");
    		printf("ERROR: Executing wait  command\n");
    		return false;
    	}
    	if (WIFEXITED (wait_status) == 0 || WEXITSTATUS (wait_status))
    	{
    		syslog(LOG_ERR,"ERROR: Child process didn't terminate properly");
    		printf("ERROR: Child process didn't terminate properly\n");
    		return false;
    	}
    
    } 
    va_end(args);
    syslog(LOG_DEBUG,"DEBUG: Normal termination of child process");
    closelog();	
    return true;
}
