/* In this example we share a memory region between
 * parent and child processes using anonymous 
 * file maps
 */
#include<iostream>

#include<sys/mman.h>
#include<semaphore.h>

#include<sys/types.h>
#include<unistd.h>

int main(){
	int * shared_queue = (int*) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 ,0);
	sem_t* consumer_lock = (sem_t*) mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	sem_init(consumer_lock, 1, 0);
	// Anonymous file maps are non-persistent, they don't need any unlinking

	int pid = fork();
	if (pid == 0){
		// Wait until the produer has started producing
		sem_wait(consumer_lock);
		for(int i =0; i < 4096/sizeof(int); i ++){
			std::cout << shared_queue[i] << '\n';
		}

	}
	else if (pid > 0){
		// Open the lock as soon as you have started producing
		sem_post(consumer_lock);
		for(int i =0; i < 4096/sizeof(int); i ++){
			shared_queue[i] = i;
		}
	}
	return 0;
}
