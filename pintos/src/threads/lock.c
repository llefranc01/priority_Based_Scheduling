
#include <stdio.h>
#include <string.h>

#include "threads/lock.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

void
lock_init(struct lock *lock)
{
    ASSERT(lock != NULL);

    lock->holder = NULL;
    semaphore_init(&lock->semaphore, 1);
}



 
bool
lock_try_acquire(struct lock *lock)
{
    ASSERT(lock != NULL);
    ASSERT(!lock_held_by_current_thread(lock));

    bool success = semaphore_try_down(&lock->semaphore);
    if (success) {
        lock->holder = thread_current();
    }
    return success;
}




void
lock_acquire(struct lock *lock)
{
    ASSERT(lock != NULL);
    ASSERT(!intr_context());
    ASSERT(!lock_held_by_current_thread(lock));
/////////////////////////////////////////////////////////////////////////
   
 bool isGreater = (lock->holder->priority < thread_current()->priority);

 
    if( lock->holder != NULL && lock->holder->priority < thread_current()->priority ){


        if(list_empty(&lock->holder->donation)){


            lock->holder->previous_priority = lock->holder->priority;
        }
        
        lock->holder->priority = thread_current()->priority;
        



        struct list_elem *front = list_begin(&lock->holder->donation);


        struct list_elem *end = list_end(&lock->holder->donation);


        struct list_elem *temp;
        temp = front;
        
    bool donation_locked = false;


        while(temp != end){
            lock->holder->took_donation = list_entry(temp, struct lock, lock_elem);
            if(lock->holder->took_donation == lock){
                donation_locked = true;
            }
            temp = list_next(temp);
        }
        if(donation_locked){
            lock->priority = thread_current()->priority;
        }else{
            list_push_front(&lock->holder->donation, &lock->lock_elem);
            lock->priority = thread_current()->priority;
        }
    }
////////////////////////////////
    semaphore_down(&lock->semaphore);
    lock->holder = thread_current();
}



void
lock_release(struct lock *lock)
{
    ASSERT(lock != NULL);
    ASSERT(lock_held_by_current_thread(lock));
    
    if(!list_empty(&lock->holder->donation) && lock->priority > thread_current()-> previous_priority){
        
        list_remove(&lock->lock_elem);
        
        if(list_empty(&lock->holder->donation)){
            thread_current()->priority = thread_current()->previous_priority;
        }else{
            struct lock *l = list_entry(list_front(&lock->holder->donation), struct    lock, lock_elem);
            thread_current()->priority = l->priority;
        }
    }
    
    lock->holder = NULL;
    semaphore_up(&lock->semaphore);
}

bool
lock_held_by_current_thread(const struct lock *lock)
{
    ASSERT(lock != NULL);
    return lock->holder == thread_current();
}

