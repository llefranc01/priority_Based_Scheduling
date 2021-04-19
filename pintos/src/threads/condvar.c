#include <stdio.h>
#include <string.h>

#include "threads/condvar.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* 
 * Initializes condition variable COND.  A condition variable
 * allows one piece of code to signal a condition and cooperating
 * code to receive the signal and act upon it. 
 */
void
condvar_init(struct condvar *cond)
{
    ASSERT(cond != NULL);
    list_init(&cond->waiters);
}


void
condvar_wait(struct condvar *cond, struct lock *lock)
{
    ASSERT(cond != NULL);
    ASSERT(lock != NULL);
    ASSERT(!intr_context());
    ASSERT(lock_held_by_current_thread(lock));

    struct semaphore waiter;
    semaphore_init(&waiter, 0);
    waiter.semaPriority = thread_current()->priority;
    list_push_back(&cond->waiters, &waiter.elem);
    lock_release(lock);
    semaphore_down(&waiter);
    lock_acquire(lock);
}

/* 
 * If any threads are waiting on COND(protected by LOCK), then
 * this function signals one of them to wake up from its wait.
 * LOCK must be held before calling this function.
 *
 * An interrupt handler cannot acquire a lock, so it does not
 * make sense to try to signal a condition variable within an
 * interrupt handler. 
 */



static bool compare_semapriority (const struct list_elem *element1, const struct list_elem *element2, void *aux UNUSED)
{
    struct semaphore *sema1 = list_entry (element1, struct semaphore, elem);


    struct semaphore *sema2 = list_entry (element2, struct semaphore, elem);
    
    struct thread *thread1 = list_entry(list_front(&sema1->waiters), struct thread, elem);
    struct thread *thread2 = list_entry(list_front(&sema2->waiters), struct thread, elem);
    
    if ( thread2->priority < thread1->priority ){
        return true;
    }
    return false;
}


void
condvar_signal(struct condvar *cond, struct lock *lock UNUSED)
{
    ASSERT(cond != NULL);
    ASSERT(lock != NULL);
    ASSERT(!intr_context());
    ASSERT(lock_held_by_current_thread(lock));

    if (!list_empty(&cond->waiters)) {


        list_sort(&cond->waiters, compare_semapriority, NULL);

        semaphore_up(list_entry(list_pop_front(&cond->waiters), struct semaphore, elem));
    }
}

/* 
 * Wakes up all threads, if any, waiting on COND(protected by
 * LOCK).  LOCK must be held before calling this function.
 *
 * An interrupt handler cannot acquire a lock, so it does not
 * make sense to try to signal a condition variable within an
 * interrupt handler. 
 */
void
condvar_broadcast(struct condvar *cond, struct lock *lock)
{
    ASSERT(cond != NULL);
    ASSERT(lock != NULL);

    while (!list_empty(&cond->waiters)) {
        condvar_signal(cond, lock);
    }
}

