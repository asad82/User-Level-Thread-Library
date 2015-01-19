

/********************************************************************************
In this test case, you need to demonstrate your threads package by implementing a 
solution to the "food services" problem. This is actually an instance of the 
better-known multiple-producer, multiple-consumer problem. 
There are N cooks (each a separate minithread) that constantly produce burgers. 
We'll assume hat each burger has a unique id assigned to it at the time it is created. 
The cooks place their burgers on a stack (at the front of the queue you built earlier
in this assignment). There are M hungry students (each a separate minithread) that 
constantly grab burgers from the stack and eat them. Each time a student eats a burger, 
she prints a message containing the id of the burger he or she just ate. Ensure, 
for health and sanity reasons, that a given burger is consumed at most once by at 
most one student. 
There is no code given to you for this test case. You have to write your own.
Represent each cook and each customer by a thread. There is a bounded buffer that can 
hold X items (burgers) that is being filled by cooks and emptied by customers.
You need to demonstrate your solution with N = 2, 3, 4 and 5, X = 4, 5 and 6
and M = 10, 15 and 20.

**************************************************************************************/
#include "queue.h"
#include "threads.h"
#include <stdio.h>
#include <stdlib.h>
#define X 6 // buffer size
#define N 4//2 // cooks
#define M 20//5 // students




int buffer[X];
int bufferIndex=0;
int globalCounter=0;
Semaphore full , empty;
Lock mutex;


void Insert_Item(int item);
int Produce_Item();
void Consume_Item(int item);
int Remove_Item();
 
// P operation decrements the value if greater than zero, if zero go to sleep / wait
// V operation increments the value
void Producer(any_ptr arg)
{

    int item;      // unique ID of the burger that will be printed

    while(1)
    {
     Semaphore_P(empty);
     lock_acquire(mutex);
     item = Produce_Item();
     Insert_Item(item);
     lock_release(mutex);
     Semaphore_V(full);
    } // end of while
  

}// end of function


void Consumer(any_ptr arg)
{
    int item;

    while(1)
    {
     Semaphore_P(full);
     lock_acquire(mutex);
     item = Remove_Item();
     Consume_Item(item);
     lock_release(mutex);
     Semaphore_V(empty);
     //Consume_Item(item);
    } // end of while

}// end of function


int Produce_Item()
{
  if(globalCounter < 50000)
  	globalCounter++ ;
  else
       globalCounter = 0;
  printf("\n%s",getThreadName());
  printf(" Produced Burger= %d",globalCounter);
  return globalCounter;
}



void Insert_Item(int item)
{
   buffer[bufferIndex++]=item;
}


int Remove_Item()
{
   int item;
   item = buffer[--bufferIndex];
   return item;
}


void Consume_Item(int item)
{
   // here we'll print the buffer ID that has been consumed
   printf("\n%s",getThreadName());
   printf(" Consumed Burger= %d",item);
   
}


void funcA(any_ptr arg)
{
 int tid;
 int i;
 char name[30];
 
 for(i=0;i<N; i++)
     {
     sprintf(name,"Producer %d",i);
     tid = t_fork(Producer,NULL,name,10);
     }
 for(i=0;i<M; i++)
     {
     sprintf(name,"Consumer %d",i);
     tid = t_fork(Consumer,NULL,name,10);
      }
    
}


int main()
{
   globalCounter = 0;
   mutex = lock_create();
   full = semaphore_create(0,"Full Semaphore");
   empty = semaphore_create(X,"Empty Semaphore");

   t_start(funcA,NULL,"A",20);// first thread

   

}
