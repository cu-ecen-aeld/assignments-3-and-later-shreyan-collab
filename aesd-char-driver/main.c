/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 *Reference: https://github.com/cu-ecen-aeld/ldd3/blob/master/scull/main.c
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/slab.h>
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Shreyan"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
	PDEBUG("open");
	struct aesd_dev *dev;
	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev;
	return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");
	/**
	 * TODO: handle release
	 */
	return 0;
}
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t value = 0;
	ssize_t offset=0;			
	int bytes_available=0;
	int bytes_read=0;
	 
	struct aesd_dev *dev = filp->private_data; 
	struct aesd_buffer_entry* buffer_entry = NULL;
	
	PDEBUG("Amount of bytes read %zu with offset %lld",count,*f_pos); 
	if (mutex_lock_interruptible(&dev->mutex1))
	{
		return -ERESTARTSYS;
	}	
	buffer_entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->buffer, *f_pos, &offset);
	if(buffer_entry == NULL)
	{
		goto exit;
	}

	bytes_available = buffer_entry->size - offset;
	bytes_read=bytes_available;
	if(count < bytes_available)
	{
		bytes_read=count;
	}
	if (copy_to_user(buf , (buffer_entry->buffptr + offset), bytes_read))
	{
		value = -EFAULT;
		goto exit;
	}
	
	*f_pos += bytes_read;
	value = bytes_read;
	exit:
			mutex_unlock(&dev->mutex1);
			return value;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t value ;
	size_t bytes_not_copied ;
	const char* removed_entry = NULL;
	
	struct aesd_dev* dev = NULL;
	dev = (filp->private_data);
	PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
	if (mutex_lock_interruptible(&dev->mutex1))
	{	
		value= -ERESTARTSYS;
		goto exit;
	}
	

	if (dev->write_entry_value.size == 0)
	{
		dev->write_entry_value.buffptr = kzalloc(count,GFP_KERNEL);
	}
	else
	{
		dev->write_entry_value.buffptr = krealloc(dev->write_entry_value.buffptr, dev->write_entry_value.size + count, GFP_KERNEL);
	}
		
		
	if (dev->write_entry_value.buffptr == NULL)
	{ 
		value = -ENOMEM;
		goto cleanup;
	}

	bytes_not_copied = copy_from_user((void *)(&dev->write_entry_value.buffptr[dev->write_entry_value.size]), buf, count);
	value=count;
	if(bytes_not_copied)
	{
		value -= bytes_not_copied;
	}
	dev->write_entry_value.size += (count - bytes_not_copied);
	

	if (strchr((char *)(dev->write_entry_value.buffptr), '\n')) 
	{

		removed_entry = aesd_circular_buffer_add_entry(&dev->buffer, &dev->write_entry_value);
       	kfree(removed_entry);
        	dev->write_entry_value.buffptr = 0;
		dev->write_entry_value.size = 0;
	}

  cleanup:
	mutex_unlock(&dev->mutex1);
  exit:
	return value;

	
}


struct file_operations aesd_fops = {
	.owner =    THIS_MODULE,
	.read =     aesd_read,
	.write =    aesd_write,
	.open =     aesd_open,
	.release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
	int err, devno = MKDEV(aesd_major, aesd_minor);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev", err);
	}
	return err;
}



int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, aesd_minor, 1,
			"aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device,0,sizeof(struct aesd_dev));

	/**
	 * TODO: initialize the AESD specific portion of the device
	 */
	aesd_circular_buffer_init(&aesd_device.buffer);
	mutex_init(&aesd_device.mutex1);
	result = aesd_setup_cdev(&aesd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}
	return result;

}

void aesd_cleanup_module(void)
{
	dev_t devno = MKDEV(aesd_major, aesd_minor);

	cdev_del(&aesd_device.cdev);

	/**
	 * TODO: cleanup AESD specific poritions here as necessary
	 */
	aesd_circular_buffer_exit(&aesd_device.buffer);
	//TODO: check how to destroy mutex;
	unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
