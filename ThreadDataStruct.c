#include "ThreadDataStruct.h"


/*
queue structure
NULL <- (pPrev:item1=HEAD:pNext) <-> (pPrev:item2:pNext) <-> (pPrev:item3=TAIL:pNext) -> NULL
*/

void threadEnqueue(Thread **head, Thread **tail, Thread *item){
	if(*head == NULL){
		*head = *tail = item;
		(*tail)->pPrev = (*head)->pNext = NULL;
	}
	else{
		(*tail)->pNext = item;
		item->pPrev = *tail;
		item->pNext = NULL;
		*tail = item;
	}
}


Thread *threadDequeue(Thread **head, Thread **tail){
	Thread *dequeueItem;

	if(*head == NULL) return NULL;
	dequeueItem = *head;
	if(dequeueItem->pNext){ //NULL <- (pPrev:item1=HEAD:pNext) <-> (pPrev:item2=TAIL:pNext) -> NULL
		*head = dequeueItem->pNext;
		(*head)->pPrev = NULL;
		dequeueItem->pNext = NULL;
	}
	else *head = *tail = NULL; //NULL <- (pPrev:HEAD=item1=TAIL:pNext) -> NULL

	return dequeueItem;
}


Thread *threadRemove(Thread **head, Thread **tail, thread_t tid){
	Thread *removeItem = *head;

	while(removeItem){
		if(removeItem->tid == tid){
			if(removeItem->pPrev) removeItem->pPrev->pNext = removeItem->pNext;
			if(removeItem->pNext) removeItem->pNext->pPrev = removeItem->pNext;
			if(removeItem == *head) *head = removeItem->pNext;
			if(removeItem == *tail) *tail = removeItem->pPrev;
			removeItem->pNext = removeItem->pPrev = NULL;
			break;
		}
		removeItem = removeItem->pNext;
	}

	return removeItem;
}

/*
list structure
(HEAD=node1) -> (node2) -> (node3) -> NULL
*/

void threadListAdd(ThreadListNode **head, ThreadListNode *item){
	if(*head == NULL)
		*head = item;
	else{
		ThreadListNode *node = *head;
		while(node->pNext)
			node = node->pNext;
		node->pNext = item;
	}
		item->pNext = NULL;
}


ThreadListNode *threadListFind(ThreadListNode *head, thread_t tid){
	ThreadListNode *target = head;
	while(target && target->tid != tid)
		target = target->pNext;
	return target;
}


ThreadListNode *threadListRemove(ThreadListNode **head, thread_t tid){
	ThreadListNode *target = *head, *tPrev = NULL;
	while(target && target->tid != tid){
		tPrev = target;
		target = target->pNext;
	}
	if(!target) return NULL;
	if(tPrev)
		tPrev->pNext = target->pNext;
	else //target == head
		*head = target->pNext;
	target->pNext = NULL;
	return target;
}
