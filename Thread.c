#include "Thread.h"
#include "Init.h"
#include "Scheduler.h"
#include "SchedCore.h"
#include "ThreadDataStruct.h"
#include <stdlib.h>
#include <signal.h>


Thread*     ReadyQHead = NULL;
Thread*     ReadyQTail = NULL;

Thread*     WaitQHead = NULL;
Thread*     WaitQTail = NULL;

int 	thread_create(thread_t *thread, thread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
        WrapperArg *wArg = malloc(sizeof(WrapperArg)); //it should be allocated dynamically to use in wrapper function
        wArg->funcPtr = start_routine; //thread handler
        wArg->funcArg = arg; //thread handler arg

        Thread *newThread = malloc(sizeof(Thread));
        //status -> __thread_to_ready2, pExitCode -> thread_exit, tid -> __WrapperThrHandler
        newThread->readyCond = (pthread_cond_t) PTHREAD_COND_INITIALIZER; //initialize condition variable
        newThread->bRunnable = 0;
        newThread->readyMutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER; //initialize mutex
        newThread->zombieCond = (pthread_cond_t) PTHREAD_COND_INITIALIZER; //initialize condition variable
        newThread->bZombie = 0;
        newThread->readyMutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER; //initialize mutex
        newThread->parentTid = pthread_self(); //set parentTid field to current thread's pthread ID
        newThread->pPrev = newThread->pNext = NULL; //initialize list pointer to NULL
        wArg->pThread = newThread;


        int retval = pthread_create(thread, NULL, __wrapperThrHandler, (void *)wArg);
	newThread->tid = *thread; //initialize child tid field
        
	//allocation and field initialization of list node
	ThreadListNode *newNode = malloc(sizeof(ThreadListNode)); //allocate list node
        newNode->pTh = newThread; //it is used for searching identifier in __getThread()
	newNode->tid = *thread; //Thread Struct Pointer

	//ADD TO THREAD LIST
        pthread_mutex_lock(&listMutex);
        threadListAdd(&ThreadListHead, newNode); //add Thread struct to thread list
        pthread_mutex_unlock(&listMutex);

        newThread->status = THREAD_STATUS_READY; //change status

	//ENQUEUE TO READY QUEUE (for the scenario: thread_create();thread_suspend())
        pthread_mutex_lock(&readyQMutex); //mutual exclusion for ready queue
        threadEnqueue(&ReadyQHead, &ReadyQTail, newThread); //push to ready queue
        pthread_mutex_unlock(&readyQMutex); //unlock

	return retval;
}


int 	thread_join(thread_t thread, void **retval)
{
	Thread *target = __getThread(thread);
	if(!target) return -1; //cannot found Thread struct according to thread id

	Thread *current = __getThread(pthread_self());
	
	sigset_t old;
	//if SIGUSR1 is arrived before masking, this thread will be enqueued to ready, and set to ready state
	sigprocmask(SIG_BLOCK, &mask_SIGUSR1, &old); //for atomic operation
	
	/*#################### GO TO BLOCK ####################*/
	current->status = THREAD_STATUS_BLOCKED; //change caller thread state to wait state
	pthread_mutex_lock(&waitQMutex);
	threadEnqueue(&WaitQHead, &WaitQTail, current); //inqueue caller thread to wait queue
	pthread_mutex_unlock(&waitQMutex);
	
	sigprocmask(SIG_SETMASK, &old, NULL); //for atomic operation
	//if pending SIGUSR1 signal is exist at this point, __thread_to_ready() will return immediately.

	//if thread received SIGUSR1 while executing __thread_to_join(), __thread_to_ready() will return immediatly (because of sleep state.)
	__thread_to_join(target); //block until child thead call thread_exit()
	
	pthread_mutex_lock(&waitQMutex);
	threadRemove(&WaitQHead, &WaitQTail, pthread_self()); //remove thread from waiting queue
	pthread_mutex_unlock(&waitQMutex);
	/*#####################################################*/

	/*#################### GO TO READY ####################*/
        current->status = THREAD_STATUS_READY; //change status to ready
       	current->bRunnable = 0; //set runnable condition to 0 -> this will be 1 by calling __thread_to_run() from scheduler
        pthread_mutex_lock(&readyQMutex); //mutual exclusion for ready Queue
        threadEnqueue(&ReadyQHead, &ReadyQTail, current); //push to ready queue
        pthread_mutex_unlock(&readyQMutex); //unlock

	pthread_mutex_lock(&(current->readyMutex));
        while(current->bRunnable == 0) //FALSE
                pthread_cond_wait(&(current->readyCond), &(current->readyMutex)); //unlock mutex and SLEEP
        pthread_mutex_unlock(&(current->readyMutex));
	/*#####################################################*/

	if(retval) *retval = target->pExitCode; //get exit code from target if retval is not NULL

	//REMOVE TARGET NODE FROM THREAD LIST
	pthread_mutex_lock(&listMutex);
	ThreadListNode* targetNode = threadListRemove(&ThreadListHead, thread);
	pthread_mutex_unlock(&listMutex);
	//DEALLOCATE TARGET NODE
	free(targetNode);
	//DEALLOCATE TARGET THREAD STRUCT
	free(target);
	return 0;
}


int 	thread_suspend(thread_t tid)
{
	if(!__getThread(tid)) return -1; //thread id validation check

	pthread_mutex_lock(&readyQMutex);
	Thread *pTh = threadRemove(&ReadyQHead, &ReadyQTail, tid);
	pthread_mutex_unlock(&readyQMutex);
	
	pTh->status = THREAD_STATUS_BLOCKED; //change state to sleep
	pthread_mutex_lock(&waitQMutex);
	threadEnqueue(&WaitQHead, &WaitQTail, pTh); //enqueue the thread to wait queue
	pthread_mutex_unlock(&waitQMutex);

	return 0;
}


int	thread_resume(thread_t tid)
{
	if(!__getThread(tid)) return -1; //thread id validation check

	pthread_mutex_lock(&waitQMutex);
	Thread *pTh = threadRemove(&WaitQHead, &WaitQTail, tid);
	pthread_mutex_unlock(&waitQMutex);

	pTh->status = THREAD_STATUS_READY; //change its staus to ready
	pthread_mutex_lock(&readyQMutex);
	threadEnqueue(&ReadyQHead, &ReadyQTail, pTh); //enqueue the thread to ready queue
	pthread_mutex_unlock(&readyQMutex);

	return 0;
}


thread_t	thread_self()
{
	return pthread_self();
}


int	thread_exit(void *retval)
{
	Thread *current = __getThread(pthread_self());
	current->pExitCode = retval;
	current->status = THREAD_STATUS_ZOMBIE;
	__thread_to_zombie(current);
	return 0;
}
