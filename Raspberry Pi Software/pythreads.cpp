#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>


#define NUM_THREADS 4

void *Task1(void *arguments){
		
  int count = 0;
  int index = *((int *)arguments);
  printf("THREAD %d: Started.\n", index);
 
  while (1)
  {
  usleep(10*1000);
  count+=1;
  printf("THREAD %d: %d\n",index ,count);
  
}
}

void *Task2(void *arguments){
		
  int count = 0;
  int index = *((int *)arguments);
  printf("THREAD %d: Started.\n", index);
 
  while (1)
  {
  usleep(10*1000);
  count+=1;
  printf("THREAD %d: %d\n",index ,count);
  
}
}

void *Task3(void *arguments){
		
  int count = 0;
  int index = *((int *)arguments);
  printf("THREAD %d: Started.\n", index);
 
  while (1)
  {
  usleep(10*1000);
  count+=1;
  printf("THREAD %d: %d\n",index ,count);
  
}
}

void *Task4(void *arguments){
		
  int count = 0;
  int index = *((int *)arguments);
  printf("THREAD %d: Started.\n", index);
 
  while (1)
  {
  usleep(10*1000);
  count+=1;
  printf("THREAD %d: %d\n",index ,count);
  
}
}
int main(void) {
  pthread_t threads[NUM_THREADS];
  int thread_args[NUM_THREADS];
  int i;
  int result_code;
  
  //create all threads one by one

    printf("IN MAIN: Creating thread 1.\n");
    thread_args[0] = 1;
    result_code = pthread_create(&threads[0], NULL, Task1, &thread_args[0]);
    assert(!result_code);
    
    printf("IN MAIN: Creating thread 2.\n");
    thread_args[1] = 2;
    result_code = pthread_create(&threads[1], NULL, Task2, &thread_args[1]);
    assert(!result_code);
    
    printf("IN MAIN: Creating thread 3.\n");
    thread_args[2] = 3;
    result_code = pthread_create(&threads[2], NULL, Task3, &thread_args[2]);
    assert(!result_code);
    
    printf("IN MAIN: Creating thread 4.\n");
    thread_args[3] = 4;
    result_code = pthread_create(&threads[3], NULL, Task4, &thread_args[3]);
    assert(!result_code);
  

  printf("IN MAIN: All threads are created.\n");
while (1);
  return 0;
}
