#include<semaphore.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<cstring>

int main(){
	int pid;
	pid = fork();
	if (pid == 0){
		// Declare a semaphore object
		sem_t *sem;
		// Call sem_open to create or open a semaphore
		sem = sem_open("SEM", O_CREAT, 0666, 1);
		sem_wait(sem);
		int fd = open("file.txt", O_CREAT| O_APPEND | O_RDWR, 0666);
		const char* message1 = "process1\n";
		write(fd, message1, strlen(message1) +1);
		close(fd);
		sem_post(sem);
	}
	else if (pid > 0){
		sem_t *sem;
		sem = sem_open("SEM", O_CREAT, 0666, 1);
		sem_wait(sem);
		int fd = open("file.txt", O_CREAT| O_APPEND | O_RDWR, 0666);
		const char* message1 = "process2\n";
		write(fd, message1, strlen(message1) +1);
		close(fd);
		sem_post(sem);
	}
	return 0;
}


		
