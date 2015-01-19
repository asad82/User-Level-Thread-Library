
#include "queue.h"
#include "stdio.h"



extern Queue q_create(char * name)
{
  
  Queue newq;
  if((newq = (Queue)malloc(sizeof(Queue_t)) ) == NULL)
  {
    printf("unable to allocate new queue\n");
    return NULL;
  }
  newq->head = newq->tail = NULL;
  newq->numObj = 0;
  strcpy(newq->name,name);
  return newq;
}

extern void q_destroy(Queue q)
{
 if(!q)
 {
    printf("q_destroy: NULL queue passed as argument\n");
    return;
 }

 free(q);

}


extern void q_insert(Queue q, any_ptr obj)
{
  queueObj_ptr newObj;
  if(!q)
  {
    printf("q_insert: NULL queue passed as argument\n");
  }

  if((newObj = (queueObj_ptr)malloc(sizeof(queueObj)) ) == NULL)
  {
    printf("q_insert: unable to allocate memory for queu obj\n");
    return;
  }

  newObj->obj = obj;
  newObj->next = NULL;


  if(q->tail)
     q->tail->next = newObj;

  q->tail = newObj;

  if(!q->head)
     q->head = newObj;

  q->numObj++;


}

extern any_ptr q_remove(Queue q)
{
queueObj_ptr temp;
any_ptr retObj;

if(!q)
  {
    printf("\nq_remove: NULL queue passed as argument\n");
    printf("\nQueue Name is = %s",q->name);
  }


  if(q->head)
  {
    temp = q->head;
  }

  q->head = q->head->next;

  q->numObj--;

  if(!q->head)
  {
    if(q->numObj != 0)
    {
      printf("queue error: head is null but number of elements is zero\n");
      return NULL;
    }

    q->tail = NULL;
  }

  retObj = temp->obj;

  free(temp);

  return retObj;

}


extern Boolean q_is_empty(Queue q)
{
  if(q->numObj == 0)
     return TRUE;

  return FALSE;

}




