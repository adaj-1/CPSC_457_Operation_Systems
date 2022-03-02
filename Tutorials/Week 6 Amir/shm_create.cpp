#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<iostream>
/*
 * In this simple example, we create a new shared memory
 * file using the shm_open function and then delete it.
 */

int main(){
	// This call will create a new file named BLISS in /dev/shm 
	int fd = shm_open("BLISS", O_RDWR | O_CREAT, 0666);
	//shm_unlink("BLISS");
	return 0;
}
