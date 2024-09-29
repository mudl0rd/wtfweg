/* Threads.h -- multithreading library
2009-03-27 : Igor Pavlov : Public domain */

#ifndef __7Z_THREADS_H
#define __7Z_THREADS_H

#include "Types.h"

#include <pthread.h>

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#include <CoreServices/CoreServices.h>
#else
#include <semaphore.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    pthread_t thread;
    BOOL created;
}
CThread;

#define Thread_Construct(p) { (p)->created = FALSE; }
#define Thread_WasCreated(p) ((p)->created)
#define Thread_Close(p) { pthread_join((p)->thread, NULL); (p)->created = FALSE; }
#define Thread_Wait(p) pthread_join((p)->thread, NULL);
typedef void *THREAD_FUNC_RET_TYPE;
#define THREAD_FUNC_CALL_TYPE
#define THREAD_FUNC_DECL THREAD_FUNC_RET_TYPE THREAD_FUNC_CALL_TYPE
typedef THREAD_FUNC_RET_TYPE (THREAD_FUNC_CALL_TYPE * THREAD_FUNC_TYPE)(void *);
WRes Thread_Create(CThread *p, THREAD_FUNC_TYPE func, LPVOID param);

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    BOOL created;
    BOOL autoreset;
    BOOL triggered;
} CEvent;

typedef CEvent CAutoResetEvent;
typedef CEvent CManualResetEvent;

#define Event_Construct(p) { (p)->created = FALSE; }
#define Event_IsCreated(p) ((p)->created)
#define Event_Close(p) { (p)->created = FALSE; }
WRes Event_Wait(CEvent *p);
WRes Event_Set(CEvent *p);
WRes Event_Reset(CEvent *p);
WRes ManualResetEvent_Create(CManualResetEvent *p, int signaled);
WRes ManualResetEvent_CreateNotSignaled(CManualResetEvent *p);
WRes AutoResetEvent_Create(CAutoResetEvent *p, int signaled);
WRes AutoResetEvent_CreateNotSignaled(CAutoResetEvent *p);

typedef struct
{
#ifdef __APPLE__
    dispatch_semaphore_t sem;
    SInt32 count;
#else
    sem_t sem;
#endif
    UInt32 maxCount;
}
CSemaphore;

#define Semaphore_Construct(p)
#define Semaphore_Close(p)
#ifdef __APPLE__
#define Semaphore_Wait(p) { DecrementAtomic(&((p)->count)); dispatch_semaphore_wait((p)->sem, DISPATCH_TIME_FOREVER); }
#else
#define Semaphore_Wait(p) { sem_wait(&((p)->sem)); }
#endif
WRes Semaphore_Create(CSemaphore *p, UInt32 initCount, UInt32 maxCount);
WRes Semaphore_ReleaseN(CSemaphore *p, UInt32 num);
WRes Semaphore_Release1(CSemaphore *p);

typedef pthread_mutex_t CCriticalSection;
WRes CriticalSection_Init(CCriticalSection *p);
#define CriticalSection_Delete(p)
#define CriticalSection_Enter(p) pthread_mutex_lock(p)
#define CriticalSection_Leave(p) pthread_mutex_unlock(p)

#ifdef __cplusplus
}
#endif

#endif
