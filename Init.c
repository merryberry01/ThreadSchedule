#include "Init.h"
#include "Thread.h"
#include "SchedCore.h"

#include <signal.h>

void Init()
{
	//in Thead.h
	ReadyQHead = ReadyQTail = NULL;
	WaitQHead = WaitQTail = NULL;

	//in SchedCore.h
	preempt_ready_queued = 0;
	sigemptyset(&mask_SIGUSR1);
	sigaddset(&mask_SIGUSR1, SIGUSR1);
	pthread_mutex_init(&readyQMutex, NULL);
	pthread_mutex_init(&waitQMutex, NULL);

	ThreadListHead = NULL;
	pthread_mutex_init(&listMutex, NULL);

	//set signal handler of SIGUSR1
	signal(SIGUSR1, __thread_to_ready);
}
