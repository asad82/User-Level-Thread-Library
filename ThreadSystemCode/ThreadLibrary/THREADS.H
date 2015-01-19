
#include <setjmp.h>
#include "queue.h"


typedef struct signal_t
{
  any_ptr val;
}signal_t;

typedef struct Lock_t
{
int shared;      // shared variable on which the caller waits if set to 1
}Lock_t, *Lock;

typedef struct Condition_t
{
int condID;          // unique ID for each condition
Queue waiters[128]; // threads waiting to be signalled
Lock lock;          // associated lock with the condition
}Condition_t, *Condition;



typedef struct Semaphore_t
{
int value;        // value of the semaphore
int semID;        // unique ID of semaphore
Queue waiters;  // blocked threads on the semaphore    
}Semaphore_t,*Semaphore;


typedef struct tcb
{
char name[30]; // optional name string of a thread, may be used for debugging
jmp_buf context;   // saved context of this thread
signal_t *sig; // signal that wakes up a waiting thread
ThreadFunc func; // function that this thread started executing
any_ptr    arg; // argument passed to this thread's ThreadFunc
int priority;   // priority of the thread
int flag;  // used to identify whether the function will be called first time or not
int tid;   // Thread identification number
int waitingForTid;   // identification number of a thread for which this thread is waiting (if any)
//
}tcb;      // 1 indicates call the function and 0 indicates restore the context

typedef struct TheadSystem
{
Queue readyQ;
tcb *currentThread;  // currently executing thread
jmp_buf lastContext; // location on which the system jumps after all threads have exited
Condition spawnerCond; // condition on which spawner threads keeps waiting

}TheadSystem;

extern void t_start(ThreadFunc f, any_ptr v, char *name,int priority);
extern Condition cond_create(Lock lock);
extern void cond_destroy(Condition cond);
extern void t_yield();
extern any_ptr t_wait(Condition cond, Lock lock);
extern void t_sig(Condition cond, any_ptr val, Lock lock);
extern int t_fork(ThreadFunc f, any_ptr v, char *name,int priority);
extern void t_exit(int val);
extern void t_join(int tid); // t_join will store the TCB in an array of struct that will wait for
                            // child to terminate
extern int t_priority();
extern void t_set_quantum(long msec);
extern void t_set_system_quantum(int quantum);
                            
extern Semaphore semaphore_create(int count,char *semName);
extern void Semaphore_V();
extern void Semaphore_P();
extern void Semaphore_destroy(Semaphore sem);
extern Lock lock_create();
extern void acquire(Lock lock);
extern void release(Lock lock);

extern char* getThreadName();
