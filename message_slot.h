#define SUCCESS 0
#include <linux/ioctl.h>
#define MSG_SLOT_CHANNEL _IOW(235, 0, unsigned int)

struct channel
{
  char last_message[128];
  unsigned long channel_id;
  int last_msg_len; 
  
};

struct channels {
	struct channel data;
	struct channels *next;
	int not_empty;
	
};
