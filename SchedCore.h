#ifndef __SCHEDCORE_H__
#define __SCHEDCORE_H__

#include "Thread.h"
#include <signal.h>

extern char preempt_ready_queued;

extern sigset_t mask_SIGUSR1;

extern pthread_mutex_t readyQMutex;
extern pthread_mutex_t waitQMutex;

typedef struct _ThreadListNode ThreadListNode;
typedef struct _ThreadListNode{
	thread_t tid;
	Thread *pTh;
	ThreadListNode *pNext;
} ThreadListNode;

extern ThreadListNode *ThreadListHead; //Thread List Head
extern pthread_mutex_t listMutex;


void *__wrapperThrHandler(void *arg);
Thread *__getThread(thread_t tid);
void __thread_to_run(Thread *pTh);
void __thread_to_ready(int signo);
void __thread_to_ready2(Thread *pTh);
void __thread_to_join(Thread *pTh);
void __thread_to_zombie(Thread *pTh);

#endif
