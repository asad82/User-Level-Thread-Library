/***************************************************************************
                          thread.c  -  description
                             -------------------
    begin                : Tue Oct 5 2004
    copyright            : (C) 2004 by Asad Ali
    email                : asad_82@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program Implements a pre-emptive user level thread library       *
 *   Programmed BY: Asad Ali                                             *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <signal.h>
#include <sys/time.h>
#include <setjmp.h>

#include "queue.h"
#include "threads.h"


//************************* Library functions declared Explicitly ***********************//
void incrementStack();
void Scheduler();
void timerHandler();
void sitBottom();
void allocateMemory();
void disableTimer();
void enableTimer();
//************************* Library functions declared Explicitly ***********************//

//************************* Variables Used in the thread System *************************//
Queue pQueue[128];
int threadLevelCount[128];

struct sigaction sa;
struct itimerval timer;
sigset_t blocksigs; 

tcb * tcbWaiting[50]; // this array will use to store tcbs that are waiting their children to terminate
int waitingIndex = 0;
int exitedThreadIDs[50];  // record of the threads that have exited is also maintained
int exitThreadIndex = 0;

// Thread Global Identifier
int gThreadID = 0;
int semaphoreID = 0;
int conditionID = 0;
int conditionWaiters[50][128];
int semaphoreWaiters[20];
int firstRun=0,threadCount=0;
int interruptInterval1 = 10000, interruptInterval2 = 10000;

jmp_buf  schedContext;
jmp_buf  sitBottomContext;

tcb *currentRunningThread,*tempTcb;
Condition conditionRecord[50];
int sysInDeadlock=0;
int f_or_c;
signal_t *pendingSignal;
Condition sysCond;
ThreadFunc userFunction;
//************************* Variables Used in the thread System *************************//



//******************* Start of Creates the Condition *******************************//
extern Condition cond_create(Lock lock)
{
 disableTimer();
  
 Condition condTemp;

 // allocate memory to the condition variable
 if((condTemp = (Condition)malloc(sizeof(Condition_t)) ) == NULL)
   {
     printf("\nunable to create new condition\n");
     return NULL;
   }

 // as we have two Queues in a condition, so allocate them memory now
 int i;
 for(i=0;i<128;i++)
    condTemp->waiters[i] = q_create("condwaiters");

 condTemp->lock = lock ;
 condTemp->condID = conditionID;                                
 // keep record of all the condition that are created so that in the end we can
 // perform deadlock detection
 conditionRecord[conditionID++]=condTemp;

 enableTimer();
 return condTemp;
}

//********************* End of Creates the Condition *******************************//


//******************* Start of Destroys the Condition *******************************//
extern void cond_destroy(Condition cond)
{
 int i;
 for(i=0;i<128;i++)
    free(cond->waiters[i]);   // delete the waiters queue

 free(cond->lock); // and the signalers queue
 free(cond);            // destroy the condition and release memory
}
//******************* End of Destroy the Condition *******************************//


void allocateMemory()
{

  if( (currentRunningThread = malloc(sizeof(tcb)) ) == NULL )
      { printf("\nCould Not allocate Memory to currentThread"); exit(0);}

  currentRunningThread->priority = -1;

  int i;
  for(i=0;i<50;i++)
     {
      tcbWaiting[i]=malloc(sizeof(tcb));
      tcbWaiting[i]->waitingForTid=0;
     }
  for(i=0;i<20;i++)
    semaphoreWaiters[i]=0;   

  int j;  
  for(i=0; i<50; i++)
     for(j=0;j<128;j++)
          conditionWaiters[i][j]=0;

                                                                                                                                  
  if( (tempTcb = malloc(sizeof(tcb)) ) == NULL )
      { printf("\nCould Not allocate Memory for tempTcb."); exit(0); }


}
//******************* End of Allocate Memory *******************************//


extern void t_start(ThreadFunc f, any_ptr v, char *name, int priority)
{
// start all threads that the system requires
// this function will start the first thread
int i=0;
// priority queue at each level.
for(i=0;i<128;i++)
{    
    pQueue[i]=q_create("PQueue");
    threadLevelCount[i]=0;
}


// allocate memory to some variable required by the thread system
allocateMemory();

firstRun=0;
// put the first thread in the ready Queue.
int ThreadID;
ThreadID = t_fork(f,v,name,priority); 

 // Install timer_handler as the signal handler for SIGVTALRM.
 memset (&sa, 0, sizeof (sa));
 sa.sa_handler = &timerHandler;
 sa.sa_flags = SA_NOMASK | SA_ONSTACK;
 sigaction (SIGVTALRM, &sa, NULL);
 // Configure the timer to expire after 10 msec
 timer.it_value.tv_sec = 0;
 timer.it_value.tv_usec = interruptInterval1;//10000;
 // ... and every 10 msec after that. 
 timer.it_interval.tv_sec = 0;
 timer.it_interval.tv_usec = interruptInterval2;//10000;
 // Start a virtual timer. It counts down whenever this process is executing.
 setitimer (ITIMER_VIRTUAL, &timer, NULL);

 // add the signal to the set so that it can be blocked later
 if (sigaddset(&blocksigs, SIGVTALRM) == -1)
	{
		perror("sigaddset error");
		return ;
	}

disableTimer();
// this function will not return untill the last thread exits.
Scheduler();

printf("\nNo more runnable threads: Exiting thread system\n");


}
//********************* End of T_START Function *******************************//


//********************* Start of t_Fork Function *******************************//

extern int t_fork(ThreadFunc f, any_ptr v, char *name,int priority)
{
  disableTimer();
  // creates a new thread
  //printf("\n Forking thread = %s",name);

  tcb *threadTcb;
  if( (threadTcb = malloc(sizeof(tcb)) ) == NULL )
  {
    printf("\nCould Not allocate Memory to threadTcb"); exit(0);
  }

  //prepare the thread control block for future.
  strcpy(threadTcb->name,name);
  threadTcb->tid = ++gThreadID;
  threadTcb->func=f;
  threadTcb->arg=v;
  threadTcb->priority=priority;
  threadTcb->flag=1;
  threadTcb->waitingForTid=0;

  // put the new thread in the ready queue.
  q_insert(pQueue[priority],(any_ptr)threadTcb);
  threadLevelCount[priority]++;

  if(currentRunningThread->priority != -1 )
    if(priority > currentRunningThread->priority) // New thread's proirity is being checked
      t_yield();  // what about the return value of t_fork        


  enableTimer();
  return (threadTcb->tid);

}// end of function
//********************* End of t_Fork Function *******************************//


//********************* Start of t_Join Function *******************************//

extern void t_join(int tid)
{
     disableTimer();
     
     int i=0,doNotJoin = 0;
     
     for(i=0;i<exitThreadIndex;i++)
            if(tid == exitedThreadIDs[i])
                 doNotJoin = 1;

     if(doNotJoin != 1)
     {            

        if(setjmp(currentRunningThread->context)==0)
        {
            currentRunningThread->flag=0;
            currentRunningThread->waitingForTid = tid;
            REPEAT:
                if(waitingIndex < 50)
                {
                    tcbWaiting[waitingIndex++] = currentRunningThread;                    
                }
                else
                {
                    waitingIndex=0;
                    goto REPEAT;
                }   
              longjmp(schedContext,1);
            
        }
        else
            ;
      }
      
  enableTimer();     
}// end of function
//******************* End of T_FORK *******************************//

//********************* Start of lock_acquire Function *******************************//

extern void lock_acquire(Lock lock)
{
	while(lock->shared) // busy waiting for lock to be released
		;

    
	if (sigprocmask(SIG_BLOCK, &blocksigs, NULL) == -1)
	{
        printf("\nError in Lock Aquire");
	}
	//printf("\nSignal Has been temporarily suspended.");
	lock->shared = 1;

}
//********************* End of lock_acquire Function *******************************//


//********************* Start of lock_release Function *******************************//
extern void lock_release(Lock lock)
{
  disableTimer();
	lock->shared = 0;
	//printf("\nSignal Has been Restored.");

	if (sigprocmask(SIG_UNBLOCK, &blocksigs, NULL) == -1)
	{
        printf("\nError in Lock release.");
	}
  enableTimer();

}
//********************* End of Lock release Function *******************************//


//********************* Start of Timer handler Function *******************************//
void timerHandler()
{
  disableTimer();
    //printf("\nEnd of Time Slice");
 if(setjmp(currentRunningThread->context)==0)
    {
     q_insert(pQueue[currentRunningThread->priority],(any_ptr)currentRunningThread);
     threadLevelCount[currentRunningThread->priority]++;
     longjmp(sitBottomContext,1);
    }
 else   
    {  //printf("\nTimer Resuming Context");
    disableTimer();
    enableTimer();//printf("\nResuming Context.");
    }

}// end of function

//********************* End of Timer Handler Function *******************************//


//********************* Start Scheduler Function *******************************//
// Manager of the whole thread system
void Scheduler() // can call it the dispatcher
{
// select a thread from the readyQueue if not empty
// then calls the respective thread function or 
// long jump to its execution context hence restoring its execution.

  if(firstRun==0)   // save the scheduler context for the first time
  {                 // the control will never ever return to this point
    firstRun=1;     // so the firstRun check is only for clarity of purpose
    if(setjmp(schedContext)==0)
        sitBottom(); // this call will allocate stack space for the thread system
    else             // and sit at the bottom of the stack never to return again
       disableTimer();//printf("\nSched Context Restored.\n");
  
  }
      
      //printf("\nIn Scheduler");
      // select a process from the ready queue and assign it to processor
      int y;
      int success=0;
      for(y=127;y>-1;y--) // scan of the ready queue is performed
        {
          if(threadLevelCount[y]!=0)
              {
               currentRunningThread = (tcb*)q_remove(pQueue[y]);
               threadLevelCount[y]--;
               success=1;
               break;
              }
        }
      // get the flag for this thread which indicates whether we should call its function
      // or restore its context.
      f_or_c =currentRunningThread->flag;
      if(success ==1 )
      {   //printf("\nFound Thread for execution");
          if(f_or_c == 1)
          {     // if flag is one then this thread never started its execution
             // ever so increment the thread count and allocate stack space
             // then call its function from within incrementStack.
              threadCount++;
              currentRunningThread->flag=0;
              incrementStack();          
          // control will never return to this point         
          }
          else
          {     // if flag is zero thread is not new born so restore its context from where
                // it left its execution                     
              longjmp(currentRunningThread->context,1);
          }
      }
  
    

}// end of function
//******************* End of Scheduler *******************************//


//********************* Start of t_yield Function *******************************//
extern void t_yield()
{   
  disableTimer();
  // save the state of the registers of the current thread when this function is called
  // in other words call setjmp here.
  // printf("\nTrying to save context of thread.");  
  // printf("\n Yielding CPU = %s",currentRunningThread->name);
 if(setjmp(currentRunningThread->context)==0)
 {   
    // make an entry into the ready queue and return the control to the
    // scheduler and let it select some other thread.
    q_insert(pQueue[currentRunningThread->priority],(any_ptr)currentRunningThread);
    threadLevelCount[currentRunningThread->priority]++;
    longjmp(schedContext,1);
 }
 else 
    { //printf("\nYield Resuming Context.");
    disableTimer();
    enableTimer();//printf("Resuming My Context = %s\n",currentRunningThread->name);
    }
 
} // end of function
//******************* End of T_YIELD *******************************//


//********************* Start of t_wait Function *******************************//
extern any_ptr t_wait(Condition cond,Lock lock)
{       
      if(lock != NULL)
          lock_release(lock);
      disableTimer();
       
     if(setjmp(currentRunningThread->context)==0)
      {
        // insert into the waiters queue for that condition.        
        q_insert(cond->waiters[currentRunningThread->priority],(any_ptr)currentRunningThread);        
        conditionWaiters[cond->condID][currentRunningThread->priority]++;        
        longjmp(schedContext,1);      
      }
      else
      {
        //printf("Resuming My Context After Wait= %s\n",currentRunningThread->name);
        lock = (Lock) currentRunningThread->arg;
        if(lock!=NULL)
           lock_acquire(lock);
        //return (any_ptr)currentRunningThread->arg;
      }
  //}// end of else

}// end of function
//******************* End of T_WAIT *******************************//


//********************* Start of t_sig Function *******************************//
extern void t_sig(Condition cond, any_ptr val,Lock lock)
{
    disableTimer();

    int i=0,found=0;
    for(i=127;i>-1;i--)
    {
        if(conditionWaiters[cond->condID][i]!=0)
               {    found = 1;
                     break;
               }
    }
    // this function will return the control back to the calling thread.
    if(found == 1)
    {
        if(!q_is_empty(cond->waiters[i]))
        { // some waiter is waiting on the condition so remove it from the waiters queue
          // and put it in the ready queue
          tempTcb = (tcb*)q_remove(cond->waiters[i]);
          conditionWaiters[cond->condID][i]--;
          tempTcb->arg= (any_ptr)lock;
          q_insert(pQueue[tempTcb->priority],(any_ptr)tempTcb);
          threadLevelCount[tempTcb->priority]++;

        }// end of if
     } 
    //printf("\nSignaling %s",getThreadName());
    enableTimer();

}//end of function
//******************* End of T_SIGNAL *******************************//


//********************* Start of t_Exit Function *******************************//
extern void t_exit(int val)
{
// actually we do not need to do any thing here this long jump will return back
// the control to the scheduler
// and eventually end the thread in the normal fashion.
  longjmp(schedContext,1);
  // remember exit means exit, so we do not need to save any context here
}
//******************* End of T_EXIT *******************************//


//********************* Start of increment stack Function *******************************//
void incrementStack()
{
// this function is the heart of the thread system. It call the thread function
// provided by the user.
// But before this it increments the stack forward by 10000 bytes from the place
// of the last thread.
// A thread counter is maintained which helps to keep track of the stack space
// of each thread, as the control is returned back to the scheduler it has no knowledge
// of the previous calls of this function, so it can overlay the stack space of previously
// running threads. So when ever this function is called threadCounter is incremented before
// hand which multiplies with 5000 resulting in the movement of this call forward by the amount
// of space occupied by previously running threads.
//  Remember still each thread occupies only 10000 bytes.
    int var[5000*threadCount];
    userFunction=currentRunningThread->func; // assign the address of user function
    enableTimer();
    userFunction(currentRunningThread->arg); // call user function    
    int i,waked = 0;

    // check if there are any waiters, if so wake them up
    for ( i = 0; i < 50; i++)
        {
            if( tcbWaiting[i]->waitingForTid !=0)
            {
                if(currentRunningThread->tid == tcbWaiting[i]->waitingForTid)
                {
                    tcb *threadTcb;
                    threadTcb = malloc(sizeof(tcb));
                    threadTcb = tcbWaiting[i];

                    q_insert(pQueue[threadTcb->priority],(any_ptr)threadTcb);
                    threadLevelCount[threadTcb->priority]++;
                    waked = 1;
                    //tcbWaiting[i] = 0; // Freeing the space
                    //break;
                }
            }
        }  
        // put it in the ready Queue 

       if(waked == 0 )  // no thread waiting
           exitedThreadIDs[exitThreadIndex++] = currentRunningThread->tid;
     
     
     longjmp(sitBottomContext,1);    // never return back from this function as this will0
    // corrupt the stack. so long jump to the scheduler and let it select some new thread to run.
    
}

//******************* End of increment Stack *******************************//

//********************* Start of sitbottom Function *******************************//                                                                                    
void sitBottom()
{
// this function never returns back. It allocates the total space for all the threads
// that can run using this thread space initially
  int array[5000*50];    // allocate the total thread space
   
  if(setjmp(sitBottomContext)==0)  // save context
     longjmp(schedContext,1);      // and long jump back to the scheduler
  else
      {
      disableTimer();
      longjmp(schedContext,1);
      }
// never return from this function.
}
//******************* End of sit at Bottom of Stack *******************************//


//********************* Start of Semaphore_create Function *******************************//
extern Semaphore semaphore_create(int count,char *semName)
{
  disableTimer();
  
  Semaphore sem;
  if((sem = (Semaphore)malloc(sizeof(Semaphore_t)) ) == NULL)
  {
    printf("unable to allocate memory for semaphore\n");
    return NULL;
  }
  
  sem->waiters = q_create(semName);
  sem->waiters->numObj=0;
  sem->value = count;
  sem->semID = ++semaphoreID;
  enableTimer();
  return sem;

}
//********************* End of Semaphore_create Function *******************************//


//********************* Start of Semaphore_P Function *******************************//
extern void Semaphore_P(Semaphore sem)
{
   disableTimer();
   if(sem->value > 0)
    {
	    sem->value--;	    
    }
   else
    {     
      //printf("\nBlocked %s",getThreadName());
    	if(setjmp(currentRunningThread->context)==0)
        {
          q_insert(sem->waiters,(any_ptr)currentRunningThread);
          semaphoreWaiters[sem->semID]++;
          longjmp(schedContext,1);
        }
        else
         {
         disableTimer();
         
	       }
    }
enableTimer();
}// end of function
//********************* End of Semaphore_P Function *******************************//

//********************* Start of Semaphore_V Function *******************************//
extern void Semaphore_V(Semaphore sem)
{
     disableTimer();
     // if any waiters just wake them and return	
     if(semaphoreWaiters[sem->semID] > 0 )
     {
        //printf("\nName of Thread = %s",currentRunningThread->name);
        tempTcb =(tcb*) q_remove(sem->waiters);
        semaphoreWaiters[sem->semID]--;
        // put the new thread in the ready queue.
        q_insert(pQueue[currentRunningThread->priority],(any_ptr)tempTcb);
        threadLevelCount[currentRunningThread->priority]++;
     }
     else
     {   // increment the variable
         sem->value++;
     }

     enableTimer();
}
//********************* End of Semaphore_V Function *******************************//


//********************* Start of Semaphore Destroy Function *******************************//
extern void Semaphore_destroy(Semaphore sem)
{
  disableTimer();
  
 if(!sem)
 {
    printf("\nNull Semaphore passed as argument.");
    return;
 }

 free(sem);	   
 enableTimer();
 
} // end of function
//********************* End of Semaphore Destroy Function *******************************//


//********************* Start of Lock Create Function *******************************//
extern Lock lock_create()
{
  disableTimer();
  Lock lock;
  if((lock = (Lock)malloc(sizeof(Lock_t)) ) == NULL)
  {
    printf("unable to allocate memory for lock\n");
    return NULL;
  }
  lock->shared = 0;
  enableTimer();
  return lock;
  
}// end of function

//********************* End of Lock create Function *******************************//


//********************* Start of GetthreadName Function *******************************//
char * getThreadName()
{  
  return currentRunningThread->name;
}
//********************* End of GetthreadName Function *******************************//


//********************* Start of DisableTimer Function *******************************//
void disableTimer()
{

  if (sigprocmask(SIG_BLOCK, &blocksigs, NULL) == -1)
  {
    printf("\nError in Disable Timer");
  }

}
//********************* End of disableTimer Function *******************************//


//********************* Start of enableTimer Function *******************************//
void enableTimer()
{

  if (sigprocmask(SIG_UNBLOCK, &blocksigs, NULL) == -1)
  {
    printf("\nError in Enable Timer");
  }

}
//********************* End of enableTimer Function *******************************//


extern int t_priority()
{
  return currentRunningThread->priority;
}


extern void t_set_system_quantum(int quantum)
{

 memset (&sa, 0, sizeof (sa));
 sa.sa_handler = &timerHandler;
 sa.sa_flags = SA_NOMASK ;
 sigaction (SIGVTALRM, &sa, NULL);
 timer.it_value.tv_sec = 0;
 timer.it_value.tv_usec = quantum;
 timer.it_interval.tv_sec = 0;
 timer.it_interval.tv_usec = quantum;

 setitimer (ITIMER_VIRTUAL, &timer, NULL);
 // sigaddset(&blocksigs, SIGVTALRM);
}


extern void t_set_quantum(long msec)
{
 msec=msec*1000;
 memset (&sa, 0, sizeof (sa));
 sa.sa_handler = &timerHandler;
 sa.sa_flags = SA_NOMASK ;
 sigaction (SIGVTALRM, &sa, NULL);
 timer.it_value.tv_sec = 0;
 timer.it_value.tv_usec = msec;
 timer.it_interval.tv_sec = 0;
 timer.it_interval.tv_usec = msec;

 setitimer (ITIMER_VIRTUAL, &timer, NULL);
 // sigaddset(&blocksigs, SIGVTALRM);

}

//********************* End of Thread System Code *******************************//

