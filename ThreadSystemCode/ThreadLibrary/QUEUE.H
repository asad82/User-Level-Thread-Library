/*****************************************************************************
 *
 * queue.h
 *
 ****************************************************************************/

#ifndef QUEUE_H
#define QUEUE_H

typedef void *any_ptr;

/* Define Boolean type, and associated constants. */
typedef int Boolean;
#define TRUE ((Boolean)1)
#define FALSE ((Boolean)0)

/* Public functions:
 *
 * Queue q_create ();
 *   Returns a newly allocated, empty queue.
 * void q_destroy (Queue q);
 *   Frees up memory taken by the queue. Afterwards, q is not a legal
 *   queue. Note that if q is not empty, and contains dynamically
 *   allocated objects, these will not get freed. Thus it is better
 *   to empty the queue via a loop before calling here.
 * void q_insert (Queue q, any_ptr obj);
 *   Inserts obj onto the end of q.
 * any_ptr q_remove (Queue q)
 *   Returns NULL if q is empty. Otherwise, removes the first object from
 *   the head of q, and returns it.
 * Boolean q_is_empty (Queue q)
 *   Returns TRUE if q has no elements, FALSE otherwise.
 *
 ****************************************************************************/
typedef struct queueObj_t
{
  any_ptr obj;
  struct queueObj_t *next;
}queueObj, *queueObj_ptr;

typedef struct Queue_t
{
 char name[20];      // every queue has some name now
 int numObj; // number of elements in the queue
 queueObj_ptr head;
 queueObj_ptr tail;
}Queue_t,*Queue;


/* External functions */
extern Queue q_create();
extern void q_destroy(Queue q);
extern void q_insert(Queue q, any_ptr obj);
extern any_ptr q_remove(Queue q);
extern Boolean q_is_empty(Queue q);
typedef void (*ThreadFunc)(any_ptr);
#endif
