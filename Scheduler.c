#include "Init.h"
#include "Thread.h"
#include "Scheduler.h"

#include "ThreadDataStruct.h"
#include "SchedCore.h"
#include <unistd.h>
#include <signal.h>


Thread *current = NULL;

int		RunScheduler( void )
{
        while(1){
		pthread_mutex_lock(&readyQMutex);
		Thread *nextThread = threadDequeue(&ReadyQHead, &ReadyQTail); //get thread to be run from ready queue
		pthread_mutex_unlock(&readyQMutex);
		__ContextSwitch(current, nextThread);
        }
}


void            __ContextSwitch(Thread *pCurThread, Thread* pNewThread)
{
	if(pNewThread){
		if(pCurThread){
			pthread_kill(pCurThread->tid, SIGUSR1); //after TIMESLICE, send signal to running thread
			//spinlock
			//waiting for enqueueing preempted thread in __thread_to_ready()
			//CAUTION: after pCurThread's bZombie is set to 1, the thread will block signal eternally, so __thread_to_ready() won't be called.
			while(preempt_ready_queued == 0 && pCurThread->status != THREAD_STATUS_ZOMBIE);
			preempt_ready_queued = 0;
//#ifdef DEBUG
//			printf("[%.3lf] send signal to tid = %ld\n", getTimeStamp(), pCurThread->tid);
//#endif
		}
		
		current = pNewThread;
	        __thread_to_run(pNewThread);
		sleep(TIMESLICE);
	}
}

