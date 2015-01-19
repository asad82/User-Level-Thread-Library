

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

int num_cars_north = 0,num_cars_south = 0;
dir cur_dir = open;
Lock my_lock; // This must be created in some main or other function
Condition cond; // This must be created in some main or other function
char dirName[5];
int inFlight=0, toggleDir=0;
int firstTime = 0;

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

    if(firstTime == 0)
     {    firstTime++;
     cur_dir = my_dir; }

   if(my_dir == north)
        num_cars = ++num_cars_north; 
   else if(my_dir == south)
        num_cars = ++num_cars_south; 

 
    while(num_cars%2==0 || cur_dir!=my_dir || inFlight >= 3 || toggleDir==1) 
    {   
        t_sig(cond,NULL,my_lock);
        t_wait(cond,my_lock);

       if(my_dir == north)
          num_cars = num_cars_north;  
       else if(my_dir == south)
          num_cars = num_cars_south;  

       if( cur_dir==my_dir && toggleDir!=1)
                  break;
                  
       if(num_cars_north == 0 || num_cars_south == 0)
       {
              if(cur_dir == north) cur_dir = south;
              else if (cur_dir == south) cur_dir = north;

              break;
       }
    }
      cur_dir = my_dir;
      inFlight++;
      if(inFlight==3 )
          toggleDir=1;      
      printf("\n%s",getThreadName());
      printf(" Arrived at %s end", getDirection(my_dir));
      lock_release(my_lock);

      t_fork(Depart,(any_ptr)my_dir,getThreadName(),10);
}

void Depart (any_ptr val)
{
   lock_acquire(my_lock);
   dir my_dir = (dir)val;

   inFlight--;
        if(inFlight==0)
         {      
            if(cur_dir==north)
              {  cur_dir = south; toggleDir = 0;  }
            else if(cur_dir==south)
              {  cur_dir = north; toggleDir = 0; } 
         }

   t_sig(cond,NULL,my_lock);
        
   if(my_dir == north)
        num_cars = --num_cars_north; 
   else if(my_dir == south)
        num_cars = --num_cars_south; 

      
   printf("\n%s",getThreadName());
   printf(" Departed from %s end", getDirection(my_dir));
   lock_release(my_lock);

}


void funcA(any_ptr arg)
{
 int tid;
 int i;
 char name[30];
 dir direction;

 firstTime = 0;
  
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

 for(i=15;i<18; i++)
     {
     sprintf(name,"Car %d",i);
     direction = 3;
     printf("\nCar %d",i);
     printf(" wants to go %s",getDirection(direction));
     tid = t_fork(Arrive,(any_ptr)direction,name,10);
     }
 

}


int main()
{
    my_lock = lock_create();
    cond = cond_create(my_lock);

    t_start(funcA,NULL,"A",10);// first thread

}


