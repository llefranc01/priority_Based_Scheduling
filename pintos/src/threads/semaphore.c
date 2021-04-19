
#include <stdio.h>
#include <string.h>

#include "threads/semaphore.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* 
 * Initializes semaphore SEMA to VALUE.  A semaphore is a
 * nonnegative integer along with two atomic operators for
 * manipulating it:
 *
 * - down or Dijkstra's "P": wait for the value to become positive, 
 * 	then decrement it.
 * - up or Dijkstra's "V": increment the value(and wake up one waiting 
 * 	thread, if any). 
 */
void
semaphore_init(struct semaphore *sema, unsigned value)
{
    ASSERT(sema != NULL);

    sema->value = value;
    list_init(&sema->waiters);
}

void
semaphore_down(struct semaphore *sema)
{
    ASSERT(sema != NULL);
    ASSERT(!intr_context());

    enum intr_level old_level = intr_disable();
    while (sema->value == 0) {
        list_push_back(&sema->waiters, &thread_current()->elem);
        thread_block();
    }
    sema->value--;
    intr_set_level(old_level);
}


static bool greater_semaPriority (const struct list_elem *elementA, const struct list_elem *elementB, void *aux UNUSED)
{
    
    struct thread *thread1 = list_entry (elementA, struct thread, elem);



    struct thread *thread2 = list_entry (elementB, struct thread, elem);
    
    if ( thread2->priority < thread1->priority ){
        return true;
    }
    return false;
}



/* 
 * Down or Dijkstra's "P" operation on a semaphore, but only if the
 * semaphore is not already 0.  Returns true if the semaphore is
 * decremented, false otherwise.
 *
 * This function may be called from an interrupt handler. 
 */
bool
semaphore_try_down(struct semaphore *semaphore)
{
    ASSERT(semaphore != NULL);

    bool success = false;
    enum intr_level old_level = intr_disable();
    if (semaphore->value > 0) {
        semaphore->value--;
        success = true;
    } else {
        success = false;
    }
    intr_set_level(old_level);

    return success;
}

/* 
 * Up or Dijkstra's "V" operation on a semaphore.  Increments SEMA's value
 * and wakes up one thread of those waiting for SEMA, if any.
 *
 * This function may be called from an interrupt handler. 
 */
void
semaphore_up(struct semaphore *semaphore)
{
    enum intr_level old_level;

    ASSERT(semaphore != NULL);

    old_level = intr_disable();

    if (!list_empty(&semaphore->waiters)) {

        list_sort (&semaphore->waiters, greater_semaPriority, NULL);

        thread_unblock(list_entry(

            list_pop_front(&semaphore->waiters), struct thread, elem));
    }
    
    semaphore->value++;
    thread_yield();    
    intr_set_level(old_level);
}

