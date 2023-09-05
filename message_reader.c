
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "message_slot.h" 

int main (int argc, char *argv[]){
	
	if (argc !=3) {
		perror("Error: Incorrect number of arguments.");
		exit(1);
	}
	
	int fd, chan_id, fail_ioctl, len_message, fail_write;
	char buffer[128];
	fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC | O_APPEND , 0600);
	if (fd ==-1){
		perror("Error: ");
		exit(1);
	}
	
	chan_id = atoi(argv[2]);
	
	fail_ioctl = ioctl(fd, MSG_SLOT_CHANNEL, chan_id);
	if (fail_ioctl ==-1){
		perror("Error: ");
		exit(1);
	}
	len_message = read(fd, buffer, 128);
	if (len_message ==-1){
		perror("Error: ");
		exit(1);
	}
	
	fail_write = write(1, buffer, len_message);
	if (fail_write ==-1){
		perror("Error: ");
		exit(1);
	}
	
	close(fd);
	
	exit(0);
}

