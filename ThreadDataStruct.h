#ifndef __THREADDATASTRUCT_H__
#define __THREADDATASTRUCT_H__

#include "SchedCore.h"
#include "Thread.h"

/*
queue structure
NULL <- (pPrev:HEAD=item1:pNext) <-> (pPrev:item2:pNext) <-> (pPrev:item3=TAIL:pNext) -> NULL
*/
void threadEnqueue(Thread **head, Thread **tail, Thread *item);
Thread *threadDequeue(Thread **head, Thread **tail);
Thread *threadRemove(Thread **head, Thread **tail, thread_t item_id);

/*
list structure
(HEAD=node1) -> (node2) -> (node3) -> NULL
*/

void threadListAdd(ThreadListNode **head, ThreadListNode *item);
ThreadListNode *threadListFind(ThreadListNode *head, thread_t tid);
ThreadListNode *threadListRemove(ThreadListNode **head, thread_t tid);

#endif
