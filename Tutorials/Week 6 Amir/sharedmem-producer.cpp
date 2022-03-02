/*
 * In this example, we have a  queue 
 * with the producer adding item to it and 
 * the consumer taking items from it. The consumer
 * is implemented in sharedmem-consumer.cpp
 */
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<iostream>

#include<unistd.h>
#include<sys/types.h>

#include<semaphore.h>

int main(){
	int shared_fd = shm_open("QUEUE", O_RDWR | O_CREAT, 0666);
	// Be careful that these number are in bytes
	ftruncate(shared_fd, 4096);
	int * shared_queue = (int*) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shared_fd, 0);
	// What's wrong with the loop condition?
	for(int i = 0 ; i < 4096/sizeof(int) ;i ++){
		shared_queue[i] = i;
	}
	
	// Without any access control, this part is going to be troublesome
	shm_unlink("QUEUE");
	munmap(shared_queue, 4096);

	/* Using access control and synchronization mechanism 
	 * we can make sure the buffer is not deleted before 
	 * consumption
	 */
	/*
	sem_t* lock = sem_open("LOCK", O_CREAT, 0666, 0);
	sem_wait(lock);
	shm_unlink("QUEUE");
	munmap(shared_queue, 4096);
	sem_unlink("LOCK");
	*/
	return 0;
}
