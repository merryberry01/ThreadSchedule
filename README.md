# Thread Round Robin Scheduler Project

## 과제 소개
Pthread Library를 이용하여 유저 레벨에서 Round Robin 기반 스레드 스케줄링 및 Pthread Wrapper API 구현한다.


## Thread 구조체 <Thread.h>
- typedef struct _Thread Thread;
- Thread ID (thread_t == pthread_t)
- Thread State: RUN, READY, BLOCKED(SLEEP), ZOMBIE
- Thread 간 이중 연결 리스트의 prev, next 포인터
- Context Switching과 Sleep을 위한 mutex와 conditional variable. boolean variable


## Thread Handler Wrapper의 인자 구조체 <Thread.h>
- typedef struct __wrapperArg WrapperArg;
- Thread Handler의 함수 포인터
- Thread Handler Argument
- Thread Handler을 실행하는 스레드의 TID (thread_t)


## Internal Functions <SchedCore.c>
### 1. void *__wrapperThrHandler(void *arg)
- **arg**: Thread Handler 관련 구조체 (WrapperArg)

1. arg에 있는 TID를 _pthread_self()_의 반환 값을 저장한다.
2. thread_to_ready2()의 호출로 CPU를 얻을 때까지 대기한다.   
3. arg에 있는 Thread Handler를 호출한다. 이때 arg에 있는 Thread Handler Argument를 전달한다.   
4. Thread Handler의 반환 값을 반환한다.


### 2. void __thread_to_run(Thread *pTh)
- **pTh**: 실행될 스레드의 Thread 구조체 포인터

1. Thread 구조체의 bRunnable 필드를 1로 설정한 후, readyCond 필드를 인자로 하여 pthread condition signal을 보낸다.


### 3. void __thread_to_ready(int signo)
- **signo**: 수신한 signal의 번호

1. Scheduler에서 Timeslice 만료 시 수신되는 SIGUSR1의 signal handler이다.
2. 해당 스레드의 구조체를 Thread List에서 찾는다.
3. 스레드의 상태를 Ready로 바꾸고, bRunnable 필드를 0으로 설정하며, Ready Queue에 추가한다.
4. bRunnable 필드가 1이 될 때까지 대기한다. 만약 __thread_to_run() 함수에서 bRunnable 필드를 1로 바꾸었다면 해당 스레드의 상태를 RUN으로 바꾼다.


### 4. void __thread_to_ready2(Thread *pTh)
- **pTh**: 생성된 스레드의 Thread 구조체 포인터

1. 해당 함수는 스레드가 생성된 시점에서만 한 번 호출된다.
2. Thread 구조체의 bRunnable 필드가 1이 될 때까지 대기한다. 만약 __thread_to_run() 함수에서 bRunnable 필드를 1로 바꾸었다면 해당 스레드의 상태를 RUN으로 바꾼다.


### 5. void __thread_to_join(Thread *pTh)
- **pTh**: 타겟 스레드의 Thread 구조체 포인터

1. 타겟 스레드의 bZombie 필드가 1이 될 때까지 기다린다.


### 6. void __thread_to_zombie(Thread *pTh)
- **pTh**: 종료 중인 스레드의 Thread 구조체 포인터

1. 해당 Thread 구조체의 bZombie 필드를 1로 설정한다.
2. zombieCond 필드를 인자로 pthread conditional signal을 전송한다.


## Scheduler <Scheduler.c>
### 1. int RunScheduler()
1. Ready Queue에서 다음에 실행할 스레드의 Thread 구조체를 가져온 후 __ContextSwitch()를 호출하여 Context Switching한다.


### 2. void __ContextSwitch(Thread *pCurThread, Thread *pNewThread)
- **pCurThread**: 기존에 실행하고 있던 스레드의 Thread 구조체
- **pNewThread**: 앞으로 실행할 스레드의 Thread 구조체

1. 해당 함수는 pNewThread가 NULL이 아닐 경우에만 Context Switching을 진행한다.
2. pCurThread가 NULL이 아니라면 pthread_kill() 호출로 해당 스레드에 SIGUSR1 시그널을 전송한다.   
3. current 전역 변수의 값을 pNewThread로 바꾸고, __thread_to_run()을 호출하여 스레드를 깨운다.   
4. TIMESLICE만큼 sleep한다.


## Thread API <Thread.c>
### 1. int thread_create(thread_t *thread, const thread_attr_t *attr, void *(*start_routing)(void *), void *arg)
- **thread**: Thread 생성 시 TID가 저장된다.
- **attr**: pthread_attr_t와 동일, 해당 과제에서는 void로 사용하지 않는다.
- **start_routine**: Thread Handler
- **arg**: Thread Handler 관련 구조체 (WrapperArg)

1. Thread 구조체를 생성한 후 각 필드를 초기화한다. 이때 bRunnable, bZombie 필드를 0으로 초기화한다.   
2. WrapperArg 구조체를 생성한 후 TID를 제외한 나머지 필드를 초기화한다.   
3. pthread_create()의 호출로 __wrapperThrHandler()을 실행하는 스레드를 생성한다.   
4. Thread List에 스레드를 추가하고, Ready 상태로 바꾸며, Ready Queue에 삽입한다.


### 2. int thread_join(thread_t thread, void **retval)
- **thread**: 타겟 스레드의 TID
- **retval**: 타겟 스레드가 호출한 thread_exit()의 인자 값

1. 호출한 스레드의 Thread 구조체를 Thread List에서 찾는다. 그리고 상태를 BLOCKED로 바꾼 후 Wait Queue에 넣는다.   
2. __thread_to_join() 함수를 호출하여 타겟 스레드의 상태가 zombie가 될 때까지 대기한다.   
3. 호출 스레드를 Wait Queue에서 제거한 후 READY 상태로 바꾼다.
4. 호출 스레드의 bRunnable 필드를 0으로 설정한 후 Ready Queue에 넣는다. 그리고 bRunnable 필드가 1이 될 때까지 대기한다.
5. 타겟 스레드의 반환 값을 매개변수 retval에 저장한다.   
6. 타겟 스레드를 Thread List에서 제거한 후 Thread 구조체의 메모리 할당을 해제한다.


### 3. int thread_suspend(thread_t tid)
- **tid**: 타겟 스레드의 TID

1. TID가 tid인 스레드를 sleep하도록 한다. tid에 해당하는 스레드가 없으면 -1을 반환한다.   
2. 타겟 스레드를 Ready Queue에서 제거한 후, 그의 상태를 BLOCKED로 바꾼다.   
3. 해당 스레드를 Wait Queue에 추가한다.


### 4. int thread_resume(thread_t tid)
- **tid**: 타겟 스레드의 TID

1. TID가 tid인 스레드를 깨운다. tid에 해당하는 스레드가 없으면 -1을 반환한다.   
2. 타겟 스레드를 Wait Queue에서 제거한 후, 그의 상태를 READY로 바꾼다.   
3. 해당 스레드를 Ready Queue에 추가한다.   


### 5. thread_t thread_self()
1. pthread_self()를 호출한다.


### 6. thread_exit(void *retval)
- **retval**: 호출 스레드의 반환 값

1. 호출 스레드의 Thread 구조체의 pExitCode 필드에 매개변수 retval의 값을 저장한다.   
2. 호출 스레드의 상태를 ZOMBIE로 바꾼다.   
__thread_to_zombie()를 호출하여 join을 호출한 스레드에 신호를 보낸다.
