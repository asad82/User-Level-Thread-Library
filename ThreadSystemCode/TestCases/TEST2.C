

/********************************************************************************
This is a simple test of your time-slicing implementation. test this code with
different values for quantum. Since all threads are infinitely looping, the
program will never terminate. A user canpress ctrl-C to terminate the program after
sifficient number of output lines hve been generated. We will do the same while
grading it

*********************************************************************************/
#include <stdio.h>


void funcB(int arg)
{
int i;
     printf("Func B\n");
	 for(;;)
	 {
	    printf("B in loop\n");
	 }

}

void funcC(int arg)
{
int i;
     printf("Func C\n");
	 for(;;)
	 {
	    printf("C in loop\n");
	 }

}

void funcD(int arg)
{
long i=0;
     printf("Func D\n");
	 for(;;)
	 {
	    printf("D in loop\n");
	    //if(i == 90000)
            //   { printf("\n calling temp Function");
	//	 tempFunction();
	//	}	
	//i++;
	 }

}

void funcE(int arg)
{
int i;
     printf("Func E\n");
	 for(;;)
	 {
	    printf("E in loop\n");
	 }

}



void funcA(int arg)
{
int tid;

tid = t_fork(funcB,NULL,"B",20);
tid = t_fork(funcC,NULL,"C",20);
tid = t_fork(funcD,NULL,"D",20);
tid = t_fork(funcE,NULL,"E",20);
printf("A terminating\n");
}




int main()
{
int tid;
tid = t_start(funcA,NULL,"A", 30);

}
