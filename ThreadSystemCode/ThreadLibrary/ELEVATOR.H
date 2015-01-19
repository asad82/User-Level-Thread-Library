#include "queue.h"
#include "threads.h"
#include <stdio.h>

#define UP 1
#define DOWN 2

#define MAX_RIDERS 5
#define MAX_FLOORS 11

typedef enum elevatorState
{
MOVING,
DOOR_CLOSED,
DOOR_OPENED, 
STOPPED
}elevatorState;

typedef struct riderInit
{
char name[30]; // rider's name
int startFloor; // floor where the rider starts
int endFloor; // floor where the rider wants to go
}riderInit;

typedef struct floor
{
Condition goingUp; // riders wait on this condition while itending to go up
Condition goingDown; // riders wait on this condition while itending to go down
int numUp; // number of riders waiting to go up
int numDown; // number of writers waiting to go down
}floor;

typedef struct elvFloor
{
Condition requests; // riders wait in the elevator on this condition for a floor
int numWaiters; // number of riders waiting for a floor
//int numWaitDirec;
}elvFloor;


typedef struct building
{
floor floors[MAX_FLOORS];
Condition elv_wait;   // elevator waits on this for riders to enter
Condition emptyElevator;
int waiters; // counter to indicate that there are non-zero waiters
int setPresident;
int setDirectors;
int setEmployee;
int reqSFP,reqEFP;
int reqSFD[3],reqEFD[3];
int reqSFE[10],reqEFE[10];
int waitingforEmpty;
//int pEntered,dEntered,eEntered;
}building;


typedef struct elevator
{
int direction; // moving up or down
int currentFloor; // What floor is elevator at right now
elevatorState state;   // current state of elevator
int numRiders; // number of riders present in the elevator right now
int presidentRiders,directorRiders,employeeRiders;
elvFloor floors[MAX_FLOORS]; // riders wait in the elevator to reach their requested floors

}elevator;

void elevatorEnter();
void elevatorExit();
void elevatorRequestFloor(int floor);
void elevatorCallUP(int floorS,int floorE);
void elevatorCallDown(int floorS,int floorE);

void el_openDoor();
void el_closeDoor();
void el_visitFloor(int,int,int); // stop and pick & drop riders
void el_moveOneFloorUp(); 
void el_moveOneFloorDown();





