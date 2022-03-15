/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @Modified by Shreyan Prabhu D
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/slab.h>
#else
#include <stdio.h>
#include <string.h>
#endif

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer. 
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
			size_t char_offset, size_t *entry_offset_byte_rtn )
{
    int iterations =0;
    int total_buffer_size= buffer->entry[buffer->in_offs].size;
    int commands_index = buffer->in_offs;
   if((buffer == NULL) ) /*Invalid argument check*/
   {
   	return NULL;
   }


   while(iterations < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)	/*Running for max iterations permissible*/
    {
    
    	if(char_offset <= (total_buffer_size - 1))			
        {
        
        	*entry_offset_byte_rtn = char_offset-(total_buffer_size - buffer->entry[commands_index].size);
         	return &buffer->entry[commands_index];
        }
          	
      	else if(char_offset == (total_buffer_size - 1))		/*When offset equals end of one command*/
      	{
      		*entry_offset_byte_rtn = char_offset;
         	return &buffer->entry[commands_index];
      	
      	}
        else 
        {
            if(++commands_index == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) /*When buffer is fully traversed*/
            {
                commands_index = 0;
            }
           
           total_buffer_size += buffer->entry[commands_index].size;
        }

        iterations++;
    }

    return NULL;

}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
const char* aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description 
    */
   const char* entry_when_full=NULL;
   if(buffer->full == false)						/*Adding command when buffer is not full*/
   {
        buffer->entry[buffer->in_offs] = *(add_entry);
        buffer->in_offs++;
   }
   else if(buffer->full == true)					/*Adding command when buffer is full*/
   {
        entry_when_full = buffer->entry[buffer->in_offs].buffptr;
   	buffer->entry[buffer->in_offs] = *(add_entry);
        buffer->out_offs++;
        buffer->in_offs++; 
         if(buffer->out_offs == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
  	 {
        	buffer->out_offs = 0;
      	 }
   }

   if(buffer->in_offs == buffer->out_offs )			
   {
        buffer->full = true;
   }
   if(buffer->in_offs == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
   {
        buffer->in_offs = 0;
   }
   return entry_when_full;

}
void aesd_circular_buffer_exit(struct aesd_circular_buffer *buffer)
{
	int i;
	struct aesd_circular_buffer *buf = buffer;			//TODO: Check if local variables are required
	struct aesd_buffer_entry *entry;
	AESD_CIRCULAR_BUFFER_FOREACH(entry,buf,i) 
	{
		if(entry->buffptr != NULL)
		{
			#ifdef __KERNEL__
				kfree(entry->buffptr);
			#else
				free((void*)entry->buffptr);	
			#endif
		}
		
	}

}
/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
    buffer->full = false;
    buffer->in_offs = 0;
    buffer->out_offs = 0;
}
