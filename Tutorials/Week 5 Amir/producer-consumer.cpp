#include<semaphore.h>
#include<pthread.h>
#include<iostream>
#include<time.h>
#include<unistd.h>

using namespace std;

class queue_node{
	public:
		queue_node* prev = nullptr;
		int value;
};

// The queue data structure
struct buffer{
	int size = 0;
	// We keep the head element for consumers
	queue_node* head = nullptr;
	// We keep the tail element for producer
	queue_node* tail = nullptr;
};

// Create an empty shared buffer
struct buffer shared_buffer;

// Create two semaphore
// Keep track of how many empty spots/ available spots exist
// inside of the queue
sem_t available;
sem_t lock;

void* produce (void* arg){
	while(true){
		// Wait for free spaces the become available inside of the queue
		sem_wait(&available);
		// Create a new queue node
		queue_node * newnode = new queue_node;
		newnode-> value = -1;
		// Wait for access to the room, or for changing the state of the queue
		sem_wait(&lock);
		if (shared_buffer.head == nullptr){
			shared_buffer.head = newnode;
		}
		else if(shared_buffer.tail == nullptr){
			shared_buffer.tail = newnode;
			shared_buffer.head->prev = shared_buffer.tail;
		}
		else{
			shared_buffer.tail->prev = newnode;
			shared_buffer.tail = newnode;
		}
		shared_buffer.size ++;
		//cout << "produced " << shared_buffer.size << "th item\n";
		cout << "PRODUCED! size of queue: " << shared_buffer.size << '\n';
		// release , unlock the room after you are done changing the state of the queue
		sem_post(&lock);
		sleep(1);
	}
	pthread_exit(0);	
}

void* consume (void* arg){
	while(true){
		// Check if the queue is available for modification
		sem_wait(&lock);
		// Check if the queue is not empty
		if ( shared_buffer.head != nullptr){
			// Consume the value of the head element
			int consumed_value = shared_buffer.head->value;
			// Make the elements behind the previous head, the new head
			shared_buffer.head = shared_buffer.head->prev;	
			shared_buffer.size --;
			//cout << "consumed " << shared_buffer.size + 1 << "th item\n";
			cout << "CONSUMED! size of queue: " << shared_buffer.size << '\n';
			sem_post(&available);
		}
		sem_post(&lock);
		sleep(1);
	}
	pthread_exit(0);
}


int main(){
	// This is for controlling the maximum size of the queue
	sem_init(&available, 0, 7);
	// This is for controlling race conditions while changing the queue
	sem_init(&lock, 0, 1);

	pthread_t producer1, producer2;
	pthread_create(&producer1, NULL, produce, NULL);
	pthread_create(&producer2, NULL, produce, NULL);

	pthread_t consumer1, consumer2;
	pthread_create(&consumer1, NULL, consume, NULL);
	pthread_create(&consumer2, NULL, consume, NULL);

	pthread_join(producer1, NULL);
	pthread_join(producer2, NULL);
	pthread_join(consumer1, NULL);
	pthread_join(consumer2, NULL);
	return 0;
}

