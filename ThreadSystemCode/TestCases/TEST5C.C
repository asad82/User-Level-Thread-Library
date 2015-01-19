

/********************************************************************************
In this test case, you will extend the elevator implementation that you corrected
in Project 1 to include priorities. Now you have three type of riders. The elevator 
is serving a large corporation's head-quarters. There is a President of the company,
three directors and 10 regular employees. When the President appears on a floor and
calls the elevator, there are the following scenarios possible:
1. The elevator is empty and stationary on a floor. In this case it must go to the
President's floor and pick him up. This is by the way true for any other rider
as well since the elevator is empty and staionary and has nothing else to do.

2. The elevator is empty but is moving in some direction to pick up a rider who 
called it before the President pressed the button. In this case the elevator must change 
its direction on the fly (if required) and go to President's floor to pick him up, thus
giving priority to President. Note that the elevator may or may not have to change
direction. If for example, it was moving UP, and is at floor 1, there are riders waiting
on floor 3, and President waiting on floor 7, it will keep moving up, not stop at 3
and continue till floor 7. After dropping off the President on his intended floor will
it go and pick up waiters on 3 provided no director appeared in the meanwhile on any
floor). If a director appeared while it was taking President to his intended floor,
it would go to that director's floor instead of going to floor 3.

3. The elevator is not empty and is either stationary or moving in certain direction. 
In this case the elevator must first drop off all the passengers on their respective 
floors before going to President's floor and picking him up. This also applies to 
directors, although President would have priority over directors. In other words, if 
there is one or more directors waiting on some floor, and there is President waiting 
on some other, the President would ride the elevator first. Similarly priority must 
be given to directors over other normal riders.

If the President is waiting on some floor and there are other riders (normal or director level)
also waiting on that same floor, only the President would be picked up. Similarly, 
if there are directors and normal riders waiting on a floor, all the directors would be 
picked up and normal riders would keep waiting.

Show that your solution works for all the above scenarios by writing different main
functions that may fork the riders, President and director threads such that different
scenarios described above are created.


**************************************************************************************/

#include <stdio.h>
#include <stdlib.h>


#include "elevator.h"


elevator elv;
building bld;
Lock lock;

void el_letOneEnter(int dir)
{
	Condition cond;
	if(dir == UP)
	{
	  cond = bld.floors[elv.currentFloor].goingUp;
	}
	else
	{
       cond = bld.floors[elv.currentFloor].goingDown;
	}

	t_sig(cond,NULL,NULL);
	t_wait(bld.elv_wait,NULL);
}

void el_letOneExit()
{
Condition cond = elv.floors[elv.currentFloor].requests;
t_sig(cond,NULL,NULL);

t_wait(bld.elv_wait,NULL);

}

void el_openDoor()
{
elv.state = DOOR_OPENED;
printf("Door opened\n");
}

void el_closeDoor()
{
elv.state = DOOR_CLOSED;
printf("Door closed\n");
}

void el_visitFloor(int numEnter,int numExit,int dir) // stop and pick & drop riders
{
  int i;

  for( i = 0; i < numEnter; i++)
  {
    el_letOneEnter(dir); //dir=UP or DOWN
  }

  for( i = 0; i < numExit; i++)
  {
    el_letOneExit();
  }



}


void ElevatorThread(any_ptr ptr)
{
int  numEnter,numExit, numSpaces;

printf("Elevator starting\n");
elv.state = STOPPED;
elv.direction = UP;
elv.currentFloor = 1;
elv.numRiders = 0;


t_wait(bld.elv_wait,NULL);


	while(1)
	{

		numSpaces = MAX_RIDERS - elv.numRiders; // number of spaces in the elevator

		if(elv.direction == UP)
		{

      numEnter = bld.floors[elv.currentFloor].numUp;
      numExit = elv.floors[elv.currentFloor].numWaiters;

      if(bld.setPresident==1 && elv.directorRiders == 0 && elv.employeeRiders == 0)
      {
        // change direction on the fly if needed.
        if(bld.reqSFP!=-1 && bld.reqSFP < elv.currentFloor)
        {       printf("Elevator direction changed on the fly.\n");
                elv.direction = DOWN;
        }       
        if(elv.currentFloor == bld.reqSFP && bld.reqEFP > elv.currentFloor)
        {
            el_openDoor();
            el_visitFloor(1,0,UP);
            el_closeDoor();
            bld.reqSFP = -1;
        }
        ////
        else if(elv.currentFloor == bld.reqSFP && bld.reqEFP < elv.currentFloor)
        {
            elv.direction = DOWN;
            el_openDoor();
            el_visitFloor(1,0,DOWN);
            el_closeDoor();
            bld.reqSFP = -1;
        }
        ////

        if(bld.reqSFP==-1 && elv.currentFloor == bld.reqEFP)
        {
            el_openDoor();
            el_visitFloor(0,1,UP);
            el_closeDoor();
            bld.reqEFP = -1;
        }

			}// end of outer if on president
      else if(bld.setPresident==0 && bld.setDirectors > 0 && elv.employeeRiders == 0 )
      {
         int i,dEnter=0,dExit=0;
         for(i=0;i<3;i++)
         {
             if(elv.currentFloor == bld.reqSFD[i] && bld.reqSFD[i] < bld.reqEFD[i] && bld.waitingforEmpty == 0)
             {       dEnter++;
                     bld.reqSFD[i] = -1;
             }       
             if(bld.reqSFD[i] == -1 && elv.currentFloor == bld.reqEFD[i])       
             {
                    dExit++;
                    bld.reqEFD[i] = -1;
             }       
         }
         
         if(dEnter >0 || dExit > 0)
         {
            el_openDoor();
            el_visitFloor(dEnter,dExit,UP);
            el_closeDoor();
         }

      } // end of outer if on director

      else if(bld.setPresident==0 && bld.setDirectors == 0 && (numEnter>0 || numExit >0) )
      {
        //lock_acquire(lock);
        if(bld.waitingforEmpty != 0)
            numEnter = 0;

        if(numEnter !=0 || numExit !=0)
        {    
          el_openDoor();

          if(numEnter > numSpaces)
              numEnter = numSpaces;
          //printf("problem with employee\n");
          el_visitFloor(numEnter,numExit,UP);
          el_closeDoor();
        }
        //lock_release(lock);
        //printf("bld.waitingforEmpty %d    \n",bld.waitingforEmpty);

      } // end of outer if on employee
      
			 if(0 == elv.numRiders && bld.waiters == 0)
			{
			   elv.state = STOPPED;
			   t_wait(bld.elv_wait,NULL);
			}

		}
		else // for DOWN
		{
           numEnter = bld.floors[elv.currentFloor].numDown;
           numExit = elv.floors[elv.currentFloor].numWaiters;

      if(bld.setPresident==1 && elv.directorRiders == 0 && elv.employeeRiders == 0)
      {
        // change direction on the fly if needed.
        if(bld.reqSFP!=-1 && bld.reqSFP > elv.currentFloor)
        {
               printf("Elevator direction changed on the fly.\n");
               elv.direction = UP;
        }
        if(elv.currentFloor == bld.reqSFP && bld.reqEFP < elv.currentFloor)
        {
            el_openDoor();
            el_visitFloor(1,0,DOWN);
            el_closeDoor();
            bld.reqSFP = -1;
        }
        ////
        else if(elv.currentFloor == bld.reqSFP && bld.reqEFP > elv.currentFloor)
        {
            elv.direction = UP;
            el_openDoor();
            el_visitFloor(1,0,UP);
            el_closeDoor();
            bld.reqSFP = -1;
        }
        ////
        if(bld.reqSFP==-1 && elv.currentFloor == bld.reqEFP)
        {
            el_openDoor();
            el_visitFloor(0,1,DOWN);
            el_closeDoor();
            bld.reqEFP = -1;
        }

			}// end of outer if on president
      else if(bld.setPresident==0 && bld.setDirectors > 0 && elv.employeeRiders == 0 )
      {
         //printf("Directors Unable to Enter ????????????\n");
         int i,dEnter=0,dExit=0;
         for(i=0;i<3;i++)
         {
             if(elv.currentFloor == bld.reqSFD[i] && bld.reqSFD[i] > bld.reqEFD[i] && bld.waitingforEmpty == 0)
             {       dEnter++;
                     bld.reqSFD[i] = -1;
             }
             if(bld.reqSFD[i] == -1 && elv.currentFloor == bld.reqEFD[i])
             {
                    dExit++;
                    bld.reqEFD[i] = -1;
             }
         }

         if(dEnter >0 || dExit > 0)
         {
            el_openDoor();
            el_visitFloor(dEnter,dExit,DOWN);
            el_closeDoor();
         }

      } // end of outer if on director

      else if(bld.setPresident==0 && bld.setDirectors == 0 && (numEnter>0 || numExit >0) )
      {
        if(bld.waitingforEmpty != 0)
            numEnter = 0;

        if(numEnter !=0 || numExit !=0)
        {
          el_openDoor();
          if(numEnter > numSpaces)
              numEnter = numSpaces;
          //printf("problem with employee %d\n",numEnter);
          el_visitFloor(numEnter,numExit,DOWN);
          el_closeDoor();
        }

      } // end of outer if on employee


       if(0 == elv.numRiders && bld.waiters == 0)
      {
          elv.state = STOPPED;
          t_wait(bld.elv_wait,NULL);
      }


    }// end of else on DOWN




      if(elv.state == DOOR_OPENED)
          el_closeDoor();


      if(bld.waiters | elv.numRiders != 0)
      {
          if(elv.direction == UP)
              el_moveOneFloorUp();  // Move to the next floor
          else
              el_moveOneFloorDown();
      }

	} // end of while

}

void el_moveOneFloorUp()
{


if(elv.state == DOOR_OPENED) // We can't move unless door is closed
   return;

elv.currentFloor++;


 t_yield();
 t_yield();
 t_yield();

 if(elv.currentFloor == MAX_FLOORS)
 {
    elv.currentFloor--;
    if(elv.direction == UP)
	{
	   elv.direction = DOWN;


	}

 }

 printf("elevator now on floor %d\n",elv.currentFloor);

}

void el_moveOneFloorDown()
{

if(elv.state == DOOR_OPENED) // We can't move unless door is closed
   return;


 elv.currentFloor--;

printf("elevator now on floor %d\n",elv.currentFloor);

 t_yield();
 t_yield();
 t_yield();

 if(elv.currentFloor == 1)
 {
    if(elv.direction == DOWN)
	{
	   //printf("elevator changing directions from DOWN to UP on floor %d\n",elv.currentFloor);
	   elv.direction = UP;
	}
 }


}

void elevatorEnter()
{

  elv.numRiders++;
  if(t_priority() == 30) 
        elv.presidentRiders++;
  else if(t_priority()==20)     
        elv.directorRiders++;
  else if (t_priority()==10)
        elv.employeeRiders++;
        
  bld.waiters--;

} // end of function

void elevatorExit()
{

  elv.floors[elv.currentFloor].numWaiters--;
  elv.numRiders--;
        

  if(t_priority() == 30)
      {   elv.presidentRiders--;
          bld.setPresident--;

          bld.reqSFP = -1;
          bld.reqEFP = -1;
          //bld.pEntered = 0;
      }
  else if (t_priority() == 20)
      {   elv.directorRiders--;
          bld.setDirectors--;
          //bld.reqSFD[bld.setDirectors] = -1;
          //bld.reqEFD[bld.setDirectors] = -1;
          //bld.dEntered--;
      }
  else if (t_priority() == 10)
      {   elv.employeeRiders--;
          bld.setEmployee--;
          bld.reqSFE[bld.setEmployee] = -1;
          bld.reqEFE[bld.setEmployee] = -1;
          //bld.eEntered--;
      }

  if(elv.employeeRiders == 0 && elv.directorRiders == 0)
      {
        int i;     // do a broadcast to notify all at once so they can proceed.
        for(i=0;i<5;i++)
            t_sig(bld.emptyElevator,NULL,NULL);
      }

  t_sig(bld.elv_wait,NULL,NULL);


}


void elevatorRequestFloor(int floor)
{
  elv.floors[floor].numWaiters++;

  t_sig(bld.elv_wait,NULL,NULL);
  t_wait(elv.floors[floor].requests,NULL);

}


void elevatorCallUP(int floorS, int floorE)
{
  if(elv.state == STOPPED)
    t_sig(bld.elv_wait,NULL,NULL);

  bld.floors[floorS].numUp++;
  bld.waiters++;

  if(t_priority() == 30)
      {
          if(elv.employeeRiders!=0 || elv.directorRiders!=0)
          {
              bld.waitingforEmpty++;
              printf("---President waiting on empty elevator\n");
              t_wait(bld.emptyElevator,NULL);
              printf("---elevator empty for president to enter\n");
              bld.waitingforEmpty--;
              bld.setPresident++;
              bld.reqSFP = floorS;
              bld.reqEFP = floorE;
          }
          else
          {
              bld.setPresident++;
              bld.reqSFP = floorS;
              bld.reqEFP = floorE;
          }
      }

  else if (t_priority() == 20)
      {
          if(elv.employeeRiders!=0)
          {
              bld.waitingforEmpty++;
              printf("---Director waiting for the elevator to be empty\n");
              t_wait(bld.emptyElevator,NULL);
              printf("---Elevator empty for director to enter\n");
              bld.waitingforEmpty--;

              bld.reqSFD[bld.setDirectors] = floorS;
              bld.reqEFD[bld.setDirectors] = floorE;
              bld.setDirectors++;
          }
          else
          {
              bld.reqSFD[bld.setDirectors] = floorS;
              bld.reqEFD[bld.setDirectors] = floorE;
              bld.setDirectors++;
          }
      }
  else if (t_priority() == 10)
      {
          bld.reqSFE[bld.setEmployee] = floorS;
          bld.reqEFE[bld.setEmployee] = floorE;
          bld.setEmployee++;
      }

  t_wait(bld.floors[floorS].goingUp,NULL);
  bld.floors[floorS].numUp--;

}

void elevatorCallDown(int floorS, int floorE)
{

  if(elv.state == STOPPED)
	    t_sig(bld.elv_wait,NULL,NULL);//TRUE);

  bld.floors[floorS].numDown++;
  bld.waiters++;
  
  if(t_priority() == 30)
      {
          if(elv.employeeRiders!=0 || elv.directorRiders!=0)
          {
              bld.waitingforEmpty++;
              printf("---President waiting on empty elevator\n");
              t_wait(bld.emptyElevator,NULL);
              printf("---elevator empty for president to enter\n");
              bld.waitingforEmpty--;
              bld.setPresident++;
              bld.reqSFP = floorS;
              bld.reqEFP = floorE;
          }
          else    
          {
              bld.setPresident++;
              bld.reqSFP = floorS;
              bld.reqEFP = floorE;
          }
      }
      
  else if (t_priority() == 20)
      {
          if(elv.employeeRiders!=0)
          {
              bld.waitingforEmpty++;
              printf("---Director waiting for the elevator to be empty\n");
              t_wait(bld.emptyElevator,NULL);
              printf("---Elevator empty for director to enter\n");
              bld.waitingforEmpty--;

              bld.reqSFD[bld.setDirectors] = floorS;
              bld.reqEFD[bld.setDirectors] = floorE;
              bld.setDirectors++;
          }
          else
          {
              bld.reqSFD[bld.setDirectors] = floorS;
              bld.reqEFD[bld.setDirectors] = floorE;
              bld.setDirectors++;
          }
      }
  else if (t_priority() == 10)
      {
          bld.reqSFE[bld.setEmployee] = floorS;
          bld.reqEFE[bld.setEmployee] = floorE;
          bld.setEmployee++;
      }


  t_wait(bld.floors[floorS].goingDown,NULL);
  bld.floors[floorS].numDown--;

}

int number = 0;
void rider(any_ptr arg)
{

  unsigned int start, end;
  char name[30];
  start = (unsigned int)arg;
  end   = (unsigned int)arg;


  start = (start & 0xffff);
  end =   (end >> 16);


  //sprintf(name,"rider%d",++number);
  printf("%s going to wait for the elevator\n",getThreadName());
  if(start < end)
  {
     //printf("Call Up\n");
     elevatorCallUP(start,end);
  }
  else if(start > end)
  {
     //printf("Call Down\n");
     elevatorCallDown(start,end);
  }

  printf("%s entering elevator on floor %d\n",getThreadName(), elv.currentFloor);
  elevatorEnter();

  printf("%s requesting floor %d\n",getThreadName(), end);
  elevatorRequestFloor(end);

  printf("%s leaving elevator on floor %d\n",getThreadName(), elv.currentFloor);
  elevatorExit();

  t_exit(0);

}

void firstThread(any_ptr ptr)
{

riderInit *riderinit;
int startFloor,endFloor, arg, i;
int riderNum = 0;


for( i = 0; i < MAX_FLOORS; i++)
{
  bld.floors[i].goingUp = cond_create(NULL);
  bld.floors[i].goingDown = cond_create(NULL);
  bld.floors[i].numUp = 0;
  bld.floors[i].numDown = 0;
}

bld.elv_wait = cond_create(NULL);
bld.emptyElevator = cond_create(NULL);
bld.waiters = 0;
bld.setDirectors = 0;
bld.setPresident = 0;
bld.setEmployee = 0;
bld.reqSFP = -1; bld.reqEFP = -1;
bld.waitingforEmpty = 0;

for( i = 0; i < MAX_FLOORS; i++)
{
  elv.floors[i].requests = cond_create(NULL);
  elv.floors[i].numWaiters = 0;
}
elv.presidentRiders = 0;
elv.directorRiders = 0;
elv.employeeRiders = 0;
elv.numRiders = 0;

t_fork(ElevatorThread,NULL,"elevator",5);


startFloor = 1;
endFloor   = 4;
startFloor |= (endFloor << 16);
//printf("Forking Low Priority employee");
t_fork(rider,(any_ptr)startFloor,"Employee1",10);

startFloor = 3;
endFloor   = 7;
startFloor |= (endFloor << 16);

//printf("\n Forking Low Priority employee");
t_fork(rider,(any_ptr)startFloor,"Employee2",10);

t_yield();

startFloor = 3;
endFloor   = 9;
startFloor |= (endFloor << 16);
//printf("\n Forking Director of High Priority.");
t_fork(rider,(any_ptr)startFloor,"Director2",20);



//printf("\n context restored\n");
startFloor = 3;
endFloor   = 8;
startFloor |= (endFloor << 16);
//printf("\n Forking President of High Priority.");
t_fork(rider,(any_ptr)startFloor,"President",30);

startFloor = 4;
endFloor   = 8;
startFloor |= (endFloor << 16);

t_fork(rider,(any_ptr)startFloor,"Employee3",10);



startFloor = 5;
endFloor   = 1;
startFloor |= (endFloor << 16);

t_fork(rider,(any_ptr)startFloor,"Employee4",10);

startFloor = 2;
endFloor   = 6;
startFloor |= (endFloor << 16);

t_fork(rider,(any_ptr)startFloor,"Employee5",10);

startFloor = 7;
endFloor   = 1;
startFloor |= (endFloor << 16);

t_fork(rider,(any_ptr)startFloor,"Employee6",10);


startFloor = 3;
endFloor   = 7;
startFloor |= (endFloor << 16);
//printf("\n Forking Director of High Priority.");
t_fork(rider,(any_ptr)startFloor,"Director1",20);

startFloor = 5;
endFloor   = 9;
startFloor |= (endFloor << 16);

t_fork(rider,(any_ptr)startFloor,"Employee7",10);

startFloor = 3;
endFloor   = 1;
startFloor |= (endFloor << 16);

t_fork(rider,(any_ptr)startFloor,"Employee8",10);

startFloor = 10;
endFloor   = 5;
startFloor |= (endFloor << 16);
//printf("\n Forking Director of High Priority.");
t_fork(rider,(any_ptr)startFloor,"Director3",20);

startFloor = 6;
endFloor   = 10;
startFloor |= (endFloor << 16);

t_fork(rider,(any_ptr)startFloor,"Employee9",10);

startFloor = 5;
endFloor   = 3;
startFloor |= (endFloor << 16);

t_fork(rider,(any_ptr)startFloor,"Employee10",10);



}


int main()
{
   lock = lock_create();
   t_start(firstThread,NULL,"firstThread",5);
}




