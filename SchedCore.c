#include "ThreadDataStruct.h"
#include <stdlib.h>

char preempt_ready_queued;

sigset_t mask_SIGUSR1;

pthread_mutex_t readyQMutex;
pthread_mutex_t waitQMutex;

ThreadListNode *ThreadListHead;
pthread_mutex_t listMutex;


void *__wrapperThrHandler(void *arg)
{
        WrapperArg *pArg = arg;

	//if child runs first, initialize tid field itself
	pArg->pThread->tid = pthread_self();

        __thread_to_ready2(pArg->pThread);
        void *retval = (pArg->funcPtr)(pArg->funcArg); //run main thread handler

        free(pArg); //free allocated wrapperArg in thread_create()
	//Thread struct will be freed by thread_join()
	//Thread List Node will be removed from list and deallocated by thread_join()
        return retval;
}


Thread *__getThread(thread_t tid)
{
        ThreadListNode *target = ThreadListHead;
        while(target && target->tid != tid)
                target = target->pNext;
        if(!target) return NULL;

        return target->pTh;
}


void __thread_to_run(Thread *pTh)
{
        pthread_mutex_lock(&(pTh->readyMutex));
        pTh->bRunnable = 1; //TRUE
        pthread_mutex_unlock(&(pTh->readyMutex));
        pthread_cond_signal(&(pTh->readyCond));
}


/* SIGUSR1 signal handler */
void __thread_to_ready(int signo)
{
        Thread *pTh = __getThread(pthread_self()); //get Thread struct of itself
	
	//if thread state is sleeping, it should not be scheduled.
	//for example, thread_join() set thread state to sleeping.
	//if thread state is ready, it will be rescheduled by itself.
	//if thread state is zombie, it will be reaped by parent.
	if(pTh->status != THREAD_STATUS_RUN){
		preempt_ready_queued = 1;
		return;
	}

        pthread_mutex_lock(&(pTh->readyMutex));

        pTh->status = THREAD_STATUS_READY; //change status to ready
        pTh->bRunnable = 0; //set runnable condition to 0 -> this will be 1 by calling __thread_to_run() from scheduler
        /* THREAD bRunnable MUST BE ZERO BEFORE SETTING THAT VARIABLE TO 1 FROM __thread_to_run() */
        /* What's the problem?: if thread is queueing before setting bRunnalbe to 0,
           then __thread_to_run() will be called and change the bRunnable to 1.
           after that, bRunnable will be set to 0 by __thread_to_ready(), and that will make infinite while loop */
        pthread_mutex_lock(&readyQMutex); //mutual exclusion for ready Queue
        threadEnqueue(&ReadyQHead, &ReadyQTail, pTh); //push to ready queue
        pthread_mutex_unlock(&readyQMutex); //unlock
	preempt_ready_queued = 1; //send cue to scheduler that preempted thread is enqueued to ready queue

        while(pTh->bRunnable == 0) //FALSE
                pthread_cond_wait(&(pTh->readyCond), &(pTh->readyMutex)); //unlock mutex and SLEEP
	pTh->status = THREAD_STATUS_RUN; //change status to run
        pthread_mutex_unlock(&(pTh->readyMutex));
}


/* this function is used for new thread */
void __thread_to_ready2(Thread *pTh)
{
        pthread_mutex_lock(&(pTh->readyMutex));
        while(pTh->bRunnable == 0) //FALSE
                pthread_cond_wait(&(pTh->readyCond), &(pTh->readyMutex));
	pTh->status = THREAD_STATUS_RUN;
        pthread_mutex_unlock(&(pTh->readyMutex));
}


/* the thread's state, which called this function, is THREAD_STATUS_BLOCKED */
/* if SIGUSR is arrived while running this logic, __thread_to_ready() will return immediately. */
void __thread_to_join(Thread *pTh)
{
        pthread_mutex_lock(&(pTh->zombieMutex));
        while(pTh->bZombie == 0)
                pthread_cond_wait(&(pTh->zombieCond), &(pTh->zombieMutex));
        pthread_mutex_unlock(&(pTh->zombieMutex));
}


void __thread_to_zombie(Thread *pTh)
{
        pthread_mutex_lock(&pTh->zombieMutex);
        pTh->bZombie = 1;
        pthread_mutex_unlock(&pTh->zombieMutex);
	//if the scenario when the exit thread got SIGUSR1 before returning __thrWrapperHandler() occured,
	//and its Thread struct is freed by thread_join(), then __getThread() called by __thread_to_reay() will return NULL.
        pthread_cond_signal(&pTh->zombieCond);
}
