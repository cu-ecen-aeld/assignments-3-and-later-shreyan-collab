#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>



// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
     openlog("Threading.c - Log messages", 0, LOG_USER);
     thread_data_t* thread_func_args = (thread_data_t *) thread_param;
     int rc= sleep(thread_func_args->wait_to_obtain_ms / 1000 );			/*Coverting milliseconds to seconds*/
     if (rc != 0)
     {
     	syslog(LOG_ERR,"ERROR: Sleep command failed before obtaining mutex");
     	ERROR_LOG("Sleep command failed before obtaining mutex");
     	thread_func_args->thread_complete_success = false;
     }
     rc = pthread_mutex_lock(thread_func_args->mutex);
     if (rc != 0)
     {
     	syslog(LOG_ERR,"ERROR: Mutex cannot be locked");
     	ERROR_LOG("Mutex cannot be locked");
     	thread_func_args->thread_complete_success = false;
     }
     rc=sleep(thread_func_args->wait_to_release_ms / 1000 );
     if (rc != 0)
     {
     	syslog(LOG_ERR,"ERROR: Sleep command failed before releasing mutex");
     	ERROR_LOG("Sleep command failed before releasing mutex");
     	thread_func_args->thread_complete_success = false;
     }
     rc = pthread_mutex_unlock(thread_func_args->mutex);
     if (rc != 0)
     {
     	syslog(LOG_ERR,"ERROR: Mutex cannot be unlocked");
     	ERROR_LOG("Mutex cannot be unlocked");
     	thread_func_args->thread_complete_success = false;
     
     }
     closelog();
     thread_func_args->thread_complete_success = true;
     return (void*)thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    openlog("Threading.c - Log messages", 0, LOG_USER);
    thread_data_t *pthreadData = (thread_data_t *) malloc (sizeof(thread_data_t));
    if(pthreadData == NULL)
    {
    	ERROR_LOG("Unable to allocate memory through malloc");
    }  
    
    pthreadData->mutex = mutex;
    pthreadData->wait_to_obtain_ms = wait_to_obtain_ms;
    pthreadData->wait_to_release_ms= wait_to_release_ms;
    
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     * 
     * See implementation details in threading.h file comment block
     */
   
     int rc = pthread_create(thread , NULL, threadfunc, pthreadData); /*threadfunc- starting point of the thread once it executes*/
     if (rc != 0)
     {
     	syslog(LOG_ERR,"ERROR: Thread could not be created");
     	ERROR_LOG("Thread could not be created");
     	closelog();
     	return false;
     	
     }
    DEBUG_LOG("Thread is created");
    closelog(); 
    return true;
}

