/**********************************************************************
This test case is the same that appeared on your sessional 1. It tests
the basic functionality of your threads priority scheme.The output
should match the output given in the solution of
sesional 1.
***********************************************************************/
#include <stdio.h>

void funcC(int arg)
{
     printf("fie ");
}


void funcB(int arg)
{
int tid;
printf("foo ");
tid = t_fork(funcC,NULL,"C",20);
printf("far ");
t_join(tid);
printf("fum ");

}

void funcA(int arg)
{
int tid;
printf("fee ");
tid = t_fork(funcB,NULL,"B", 10); // The new thread A is being
                                 //created at a priority level 10
                                 //and it would run funcA. FuncA
                                 //takes no arguments

printf("foe ");
t_join(tid);
printf("fun\n");
}


int main()
{

t_start(funcA,NULL,"A",0);

}

