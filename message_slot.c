//copied the define, undefine from the example in rec6.
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include "message_slot.h" 

MODULE_LICENSE("GPL");



//will hold for each minor a linked list of nodes that describe channels and the last message sent on the channel.
struct channels minors_channels[257];



static int device_open( struct inode* inode, struct file* file)
{

	return SUCCESS;
}


static ssize_t device_read( struct file* file, char __user* buffer, size_t length, loff_t* offset )
{
	int i;
	char message[128];
	unsigned long left_copy;
	int last_len;
	struct channels *curr_chan;
	
	curr_chan = file->private_data;
	
	if (curr_chan == NULL){ //no ioctl
		return -EINVAL;
	}
	if ((curr_chan->data).last_msg_len == 0){ //no last message
		return -EWOULDBLOCK;
	}
	
	last_len = (curr_chan->data).last_msg_len;
	
	
	if ( last_len > length){// not enough space
		return -ENOSPC;
	}
	
	for (i=0; i< length; i++){
  		message[i]= (curr_chan->data).last_message[i];
  	}
	
	left_copy = copy_to_user(buffer, message, last_len);
	if ( left_copy != 0){
		return -EFAULT;
	}
	
	return (curr_chan->data).last_msg_len;
}


static ssize_t device_write( struct file* file, const char __user* buffer, size_t length, loff_t* offset)
{
	int i;
	char message[128];
	struct channels *curr_chan;
	curr_chan = file->private_data;
	if ((length ==0) || (length >128 )){
		return -EMSGSIZE;
	}
	
	
	if ((file->private_data == NULL) || (buffer == NULL)){
		return -EINVAL;
	}
	
 	for( i = 0; i < length; i++ ){
    	get_user(message[i], &buffer[i]);
   	}
 	
  	if (length == i){//success, putting message into the channels last message attribute.
  		
  		for (i=0; i< length; i++){
  			(curr_chan->data).last_message[i] = message[i];
  		}
  		(curr_chan->data).last_msg_len = length;
  		
  		return length;
  	}
  	else { //writes should be atomic.
		return -EINVAL;
	}
	return 0;
}


static long device_ioctl( struct file* file, unsigned int command, unsigned long chan_id)
{
	int minor;
	struct channels *new_chan;
	int found =0;
	struct channels *curr_chan;
	struct channels *prev_chan;

  	if(( MSG_SLOT_CHANNEL != command) || (chan_id ==0)){
  		return -EINVAL;
  	}
	
	minor = iminor(file->f_inode);
  	
  	curr_chan = &minors_channels[minor];
  	prev_chan = NULL;
  	
  	if (minors_channels[minor].not_empty == 0){ //this message slot has no channels, creating node for this channel.
  		new_chan = (struct channels*)kmalloc(sizeof(struct channels), GFP_KERNEL);
  		(new_chan->data).channel_id = chan_id; 
  		new_chan->next = NULL;
  		new_chan->not_empty = 1;
  		minors_channels[minor] = *(new_chan);
  		file->private_data = &minors_channels[minor]; //saving the channel.
  	}
  	
  	else {//this message slot does have channels
  	
  		while ((curr_chan != NULL) && (found==0)){
  			
  			if ((curr_chan->data).channel_id == chan_id){
  				found =1;
  			}
  			
  			else{
  				prev_chan = curr_chan;
  				curr_chan = curr_chan->next;
  			}
  		}
  		if (found ==0){ //the message slot hasn't opened this channel yet, creating node for it and adding to list.
  			new_chan = (struct channels*)kmalloc(sizeof(struct channels), GFP_KERNEL);
  			(new_chan->data).channel_id = chan_id; 
  			new_chan->next = NULL;
  			prev_chan->next = new_chan;
  			file->private_data = new_chan; //saving the channel.
  		}
  		
  		else{ //found the channel, saving it.
  			file->private_data = curr_chan;
  		}
  	}
  	
	
  	return SUCCESS;
}

static int device_release( struct inode* inode, struct file*  file)
{
	
	return SUCCESS;
}

//SETUP
struct file_operations Fops =
{
  .owner = THIS_MODULE, .read = device_read, .write = device_write, .open = device_open, .unlocked_ioctl = device_ioctl, .release = device_release,};


static int __init dev_init(void)
{
	
  	int rc = -1;
  	
  	rc = register_chrdev( 235, "message_slot", &Fops );
	
  	if( rc < 0 )
  	{
    	printk("registraion failed\n");
    	return rc;
  	}
  	
	
  	printk( "Registerated successfully ");
  	return 0;
}


static void __exit dev_cleanup(void)
{
  int i;
  struct channels *curr_chan;
  struct channels *next_chan;
  for (i=0; i<257; i++){
  	curr_chan = &minors_channels[i];
  	if (curr_chan->not_empty !=0 ){
  		while (curr_chan->next !=NULL){
  			next_chan = curr_chan->next;
  			
  			kfree(curr_chan);
  			curr_chan = next_chan;
  		}
  		
  		kfree(curr_chan);
  	}
  }
  unregister_chrdev(235, "message_slot");
}


module_init(dev_init);
module_exit(dev_cleanup);

