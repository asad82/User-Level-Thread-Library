

/*
A Bridge is undergoing repairs and only one lane is open for traffic. 
To prevent accidents, traffic lights have been installed at either end of 
the bridge to synchronize the traffic going in different directions. 
A car can only cross the bridge if there are no cars going the opposite 
direction on the bridge. Sensors at either end of the bridge detect when 
cars arrive and depart from the bridge, and these sensors control the traffic lights.
Below is a skeleton implementation of two routines, Arrive() and Depart(). 
You may assume that each car is represented by a thread, and threads call Arrive() 
when they arrive at the bridge and Depart() when they leave the bridge. 
Threads pass in their direction of travel as input to the routines.
*/
// this demonstrates that starvation can occur if there is a continious supply of threads in
// one direction.

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "threads.h"


int num_cars = 0;
typedef enum dir
{
    open = 1,
    north,
    south
}dir;

void Depart(any_ptr val);


dir cur_dir = open;
Lock my_lock; // This must be created in some main or other function
Condition cond; // This must be created in some main or other function
char dirName[5];

char* getDirection(dir d)
{

    if(d==1)
        strcpy(dirName,"open");
    else if(d==2)
        strcpy(dirName,"north");
    else if(d==3)
        strcpy(dirName,"south");

     return dirName;
}


void Arrive (any_ptr val)
{
      
    dir my_dir = (dir)val;
	  lock_acquire(my_lock);
      while (cur_dir != my_dir && cur_dir != open)
	  {   
         t_wait(cond,my_lock);
	  }
      num_cars++;
      cur_dir = my_dir;
      printf("\n%s",getThreadName());
      printf(" Arrived at %s", getDirection(my_dir));
      lock_release(my_lock);
      t_fork(Depart,(any_ptr)my_dir,getThreadName(),10);
}


void Depart (any_ptr val)
{
   dir my_dir = (dir)val;
   lock_acquire(my_lock);
   num_cars--;
   if (num_cars == 0)
      {
        cur_dir = open;
    int i;
    for(i=0;i<30;i++)   // perform a broadcast waking all blocked cars / threads.
        t_sig(cond,NULL,my_lock);
	  }
   printf("\n%s",getThreadName());
   printf(" Departed from %s",getDirection(my_dir));   
   lock_release(my_lock);

}


void funcA(any_ptr arg)
{
 int tid;
 int i;
 char name[30];
 dir direction;
 
 for(i=0;i<3; i++)
 {
     sprintf(name,"Car %d",i);
     direction = 2;
     printf("\nCar %d",i);
     printf(" wants to go %s",getDirection(direction));     
     tid = t_fork(Arrive,(any_ptr)direction,name,10);
 }
     
 for(i=3;i<6; i++)
 {
     sprintf(name,"Car %d",i);
     direction = 3;
     printf("\nCar %d",i);
     printf(" wants to go %s",getDirection(direction));
     tid = t_fork(Arrive,(any_ptr)direction,name,10);
 }

 for(i=6;i<15; i++)
 {
     sprintf(name,"Car %d",i);
     direction = 2;
     printf("\nCar %d",i);
     printf(" wants to go %s",getDirection(direction));
     tid = t_fork(Arrive,(any_ptr)direction,name,10);
 }
          
/*
 for(i=0;i<5; i++)
     {
     sprintf(name,"Thread %d",i);
     direction = 2+rand()%2;
     tid = t_fork(Arrive,(any_ptr)direction,name,10);
     }
*/

}


int main()
{
    
    my_lock = lock_create();
    cond = cond_create(my_lock);

    t_start(funcA,NULL,"A",10);// first thread

}

