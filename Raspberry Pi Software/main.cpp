#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "camera.h"
#include "ball.h"
#include "goal.h"
#include "com.h"




int main(void) {
  pthread_t threads[4];
  int thread_args[4];
  int i;
  int result_code;
  
  //create all threads one by one

    printf("IN MAIN: Creating thread 1.\n");
    thread_args[0] = 1;
    result_code = pthread_create(&threads[0], NULL, cameraTask, &thread_args[0]);
    assert(!result_code);
    
    printf("IN MAIN: Creating thread 2.\n");
    thread_args[1] = 2;
    result_code = pthread_create(&threads[1], NULL, ballTask, &thread_args[1]);
    assert(!result_code);
    
    printf("IN MAIN: Creating thread 3.\n");
    thread_args[2] = 3;
    result_code = pthread_create(&threads[2], NULL, goalTask, &thread_args[2]);
    assert(!result_code);
    
    printf("IN MAIN: Creating thread 4.\n");
    thread_args[3] = 4;
    result_code = pthread_create(&threads[3], NULL, comTask, &thread_args[3]);
    assert(!result_code);
  

  printf("IN MAIN: All threads are created.\n");
while (1);
  return 0;
}