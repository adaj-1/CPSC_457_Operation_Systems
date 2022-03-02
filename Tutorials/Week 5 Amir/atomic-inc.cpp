#include<pthread.h>
#include<iostream>
#include<vector>
#include<atomic>

using namespace std;

#define NUM_THREADS 4
// When we define an object as atomic, all of the 
// operations on that object are going to be done
// atomically. 
atomic<int> shared_variable = 0;

void * inc(void * arg){
	for(int i =0; i < 10000; i ++){
		shared_variable ++;
	}
	pthread_exit(0);
}

int main(){
	vector<pthread_t> tids;
	for (int i =0; i < NUM_THREADS; i ++){
		// Create NUM_THREADS threads
		pthread_t tid;
		pthread_create(&tid, NULL, inc, NULL);
		tids.push_back(tid);
	}
	for(int i =0; i < tids.size(); i ++){
		// Join all of the threads
		pthread_join(tids.at(i), NULL);
	}
	// Everything going according to plan, what should the output value be?
	cout << shared_variable << endl;
	return 0;
}
