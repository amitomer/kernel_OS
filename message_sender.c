
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message_slot.h" 

int main (int argc, char *argv[]){
	
	if (argc !=4) {
		perror("Error: Incorrect number of arguments.");
		exit(1);
	}
	
	int fd, chan_id, fail_ioctl, fail_write, message_len;
	
	fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC | O_APPEND , 0600);
	if (fd ==-1){
		perror("Error: ");
		exit(1);
	}
	
	chan_id = atoi(argv[2]);
	
	message_len = strlen(argv[3]);
	fail_ioctl = ioctl(fd, MSG_SLOT_CHANNEL, chan_id);
	if (fail_ioctl ==-1){
		perror("Error: ");
		exit(1);
	}
	fail_write = write(fd, argv[3], message_len);
	if (fail_write ==-1){
		perror("Error: ");
		exit(1);
	}
	
	close(fd);
	exit(0);
}

