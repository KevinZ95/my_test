// Mutual exclusion spin locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

void
initlock(struct spinlock *lk, char *name)
{
	lk->name   = name;
	lk->locked = 0;
	lk->cpu    = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void
acquire(struct spinlock *lk)
{
	pushcli(); // disable interrupts to avoid deadlock.
	if (holding(lk)) panic("acquire");

	// The xchg is atomic.
	while (xchg(&lk->locked, 1) != 0)
		;

	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that the critical section's memory
	// references happen after the lock is acquired.
	__sync_synchronize();

	// Record info about lock acquisition for debugging.
	lk->cpu = mycpu();
	getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
void
release(struct spinlock *lk)
{
	if (!holding(lk)) panic("release");

	lk->pcs[0] = 0;
	lk->cpu    = 0;

	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that all the stores in the critical
	// section are visible to other cores before the lock is released.
	// Both the C compiler and the hardware may re-order loads and
	// stores; __sync_synchronize() tells them both not to.
	__sync_synchronize();

	// Release the lock, equivalent to lk->locked = 0.
	// This code can't use a C assignment, since it might
	// not be atomic. A real OS would use C atomics here.
	asm volatile("movl $0, %0" : "+m"(lk->locked) :);

	popcli();
}

// Record the current call stack in pcs[] by following the %ebp chain.
void
getcallerpcs(void *v, uint pcs[])
{
	uint *ebp;
	int   i;

	ebp = (uint *)v - 2;
	for (i = 0; i < 10; i++) {
		if (ebp == 0 || ebp < (uint *)KERNBASE || ebp == (uint *)0xffffffff) break;
		pcs[i] = ebp[1];         // saved %eip
		ebp    = (uint *)ebp[0]; // saved %ebp
	}
	for (; i < 10; i++) pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int
holding(struct spinlock *lock)
{
	return lock->locked && lock->cpu == mycpu();
}


// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void
pushcli(void)
{
	int eflags;

	eflags = readeflags();
	cli();
	if (mycpu()->ncli == 0) mycpu()->intena = eflags & FL_IF;
	mycpu()->ncli += 1;
}

void
popcli(void)
{
	if (readeflags() & FL_IF) panic("popcli - interruptible");
	if (--mycpu()->ncli < 0) panic("popcli");
	if (mycpu()->ncli == 0 && mycpu()->intena) sti();
}


// Module 3, is here !!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Module 3 is very similar to Hw_5
// we need a lock_table to store all locks


struct {
                                             // my_lock_table
                                             // to store all the locks
    struct mutex_lock  lock_List[16];       // the size of the array to store locks
    
} my_lock_table;


// we need to check the names of locks from my_lock_table
// I just copy functions from ulib.c
// and I cannot include ulib.c because that file is in user level
// So that I just copy two function, for string-copy and string-compare

/*
char *strcpy(char *s, char *t) {
    char *os;
    os = s;
    while ((*s++ = *t++) != 0) ;
    return os;
}

int strcmp(const char *p, const char *q) {
    while (*p && *p == *q) p++, q++;
    return (uchar)*p - (uchar)*q;
}

*/
// give the lock id to the new lock
int nextLockID = 1;


// create mutex_lock
// the basic idea is same as allocate process
// but we are allocating mutex_lock in my_lock_table, rather than in proc_table
//

int
mutex_create(char *name)
{
    struct mutex_lock *curLock;     // current pointer to interate in my_lock_table
    
    acquire(&my_lock_table.lock_List);
    
    // to iterate my_lock_table, if our table already contains the name
    for (curLock = my_lock_table.lock_List; curLock < &my_lock_table.lock_List[16]; curLock++) {
        if (strcmp(curLock->name, name) == 0) {
            // so the name of a mutex_lock is existing in our table;
            curLock->owner = curLock->owner + 1;    // adding one owner to the curLock
            return curLock->lockID;
        }
    }
    
    // now we have already go through my_lock_table
    // and we can be sure that the lock we want to create is not on the table
    
    // so we need to add the mutex_lock to our table
    
    // to interate my_lock_table, to find out the first unused lock
    for (curLock = my_lock_table.lock_List; curLock < &my_lock_table.lock_List[16]; curLock++) {
        if (curLock->status == 0) {
            // so this is the first mutex_lock in our table which is empty.
            goto initilizeLock;         // to initilize a new mutex_lock
        }
        
        // otherwise, our table is full
        // we fail to allocate new mutex_lock
        return -1;                      // return -1
    }
    
    // we copy information to our "curLock"
    // and we define basic information
initilizeLock:
    strcpy(curLock->name, name);        // copy name
    curLock->status = 1;                // 1 - being used
    curLock->owner = 1;             // having 1 owner
    curLock->locked = 0;                // unlocked
    curLock->lockID = nextLockID + 1;
    
    struct proc *curProc = myproc();    // the current process
    
    // 1 process can have multiple locks
    // 1 lock can be used on multiple process
    // either a single process has 16 locks or my_lock_list has 16 locks
    // needs to return -1
    
    if (curProc->lockNums == 16) {      // curProc' locks are full
        return -1;                      // return -1 here
    }
    

    int tempIndex = curProc->lockNums;
    curProc->mutexLocks[tempIndex] = curLock->lockID;   // assign lock-id to process lock list
    curProc->lockNums = curProc->lockNums + 1;
    
    release(&my_lock_table.lock_List);
    return curLock->lockID;
}


// to delete a mutex_lock
// we need to make sure no other process still contains curLock

void
mutex_delete(int muxid)
{
    struct mutex_lock *curLock;         // current pointer to interate in my_lock_table
    
    acquire(&my_lock_table.lock_List);
    
    for (curLock = my_lock_table.lock_List; curLock < &my_lock_table.lock_List[16]; curLock++) {
        if (curLock->lockID == muxid) {
            // here we have found the target mutex_lock
            goto deleting;              // to delete a mutex_lock
        } else {
            // otherwise, we did not find such mutex_lock
            panic("Cannot Find Such Lock!");
            return;                     // just return
        }
    }
    
deleting:
    // now
    // we have two things to do
    // 1. in my_lock_table : delete target lock from our table
    // 2. in current process : remove all locks behind target lock forward
    

    // lets do 1.
    curLock->owner = curLock->owner - 1;
    
    if (curLock->owner == 0) {
        // here our target lock has no owner
        
        curLock->status = 0;
        // then we need to set its status as unused
        // so that we can create a new lock when we call mutex_create() function
        curLock->lockID = 0;
        // also reset lock_id
    }
    
    // now we can do 2.
    
    struct proc *curProc = myproc();      // a pointer to current process
    int i;
    i = 0;
    while (i < curProc->lockNums) {
        // serach current process's lock_list
        if (curProc->mutexLocks[i] == muxid) {
            // in mutex_create, we store lock_id in process's lock_list
            // here we have found the current lock
            break;                      // break
        }
        i = i + 1;
    }
    
    if (i == curProc->lockNums) {
        panic("Current Process Does NOT Contain Such Lock");
        return;
    }
    
    int j;
    for (j = i; j < curProc->lockNums - 1; j++) {
        curProc->mutexLocks[j] = curProc->mutexLocks[j+1];
    }
    curProc->lockNums = curProc->lockNums - 1;
    
    release(&my_lock_table.lock_List);
    return;
}


// the point is, if no other process is using such mutex_lock
// then we need to protect this process(thread)
// otherwise, if the mutex_lock is being used by another process(thread)
// we need to wait until other process(thread) to release such mutex_lock

void
mutex_lock(int muxid)
{
    // same way to find out our target lock in my_lock_tabe
    struct mutex_lock *curLock;
    acquire(&my_lock_table.lock_List);
    for (curLock = my_lock_table.lock_List; curLock < &my_lock_table.lock_List[16]; curLock++) {
        if (curLock->lockID == muxid) {
            break;              // we have found the lock in my_lock_table
        }
    }
    release(&my_lock_table.lock_List);
    // if the mutex_lock is not locked
    // this process(thread) can use crtical section
    // otherwise, if the mutex_lock is being locked by other process(thread)
    // this process(thread) has to wait
    
    
    // the concept is
    
    
     /*
     if (curLock->locked != 1) {
        curLock->locked = 1;  // setting the mutex_lock as locked
        return;
     } else {
        while (1) {
            cv_wait(muxid);                 // and waiting
        }
     }
    */
    // if there are two processes(threads) "at the same time" using critical section
    // they will assume mutex_lock is not locked at beginning
    // so we need to atomically set mutex_lock at first
    
    
    while (1) {
    	xchg(&curLock->locked, 1);	// atomically setting mutex_lock to lock
    
    	if (curLock->locked == 1 ) {    // if mutex_lock is locked
            continue;                 // just wait here
		
    	} else {
		return; 	
	}
   }
    
    /*
     To execute those two lines atomically, xv6 relies on a special 386 hardware in- struction, xchg (0569).
     In one atomic operation, xchg swaps a word in memory with the contents of a register.
     The function acquire (1574) repeats this xchg instruction in a loop; each iteration reads lk->locked and atomically sets it to 1 (1583).
     If the lock is held, lk->locked will already be 1, so the xchg returns 1 and the loop continues.
     If the xchg returns 0, however, acquire has successfully acquired the lock—locked was 0 and is now 1—so the loop can stop.
     Once the lock is acquired, acquire records, for debugging, the CPU and stack trace that acquired the lock.
     When a process acquires a lock and forget to release it, this information can help to identify the culprit.
     These debugging fields are protected by the lock and must only be edited while holding the lock.
     
     The function release (1602) is the opposite of acquire: it clears the debugging fields and then releases the lock.
     
     from xv6
     */

    
}


void
mutex_unlock(int muxid)
{
    struct mutex_lock *curLock;
    acquire(&my_lock_table.lock_List);
    for (curLock = my_lock_table.lock_List; curLock < &my_lock_table.lock_List[16]; curLock++) {
        if (curLock->lockID == muxid) {
            break;              // we have found the lock in my_lock_table
        }
    }
    
    release(&my_lock_table.lock_List);
    xchg(&curLock->locked, 0); // atomically setting mutex_lock to unlock
    return;
 }
    
/*
struct {
	struct spinlock lock;
	struct proc     proc[NPROC];
} ptable;
// process table
void
cv_wait(int muxid)
{
    struct mutex_lock *curLock;
    for (curLock = my_lock_table.lock_List; curLock < &my_lock_table.lock_List[16]; curLock++) {
        if (curLock->lockID == muxid) {
            break;              // we have found the lock in my_lock_table
        }
    }
    
    struct proc *curProc = myproc();
    int i;
    for (i = 0; i < curProc->lockNums; i++) {
        if (curProc->mutexLocks[i] == muxid) {
            break;
        }
    }
    
    
    // copy from yield() in proc.c
    // simply to give up the CPU for one scheduling round.
    // p is our curProc
    // lk is our &curLock->myLock
    
    acquire(&ptable.lock); // DOC: yieldlock
    myproc()->state = RUNNABLE;
    sched();
    release(&ptable.lock);
    
    
    // copy from sleep() in proc.c
    // simply to let the current process(thread) to sleep
    // p is our curProc
    // lk is our &curLock->myLock
    
    
    
    // Must acquire ptable.lock in order to
    // change p->state and then call sched.
    // Once we hold ptable.lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup runs with ptable.lock locked),
    // so it's okay to release lk.
    if (&curLock->myLock != &ptable.lock) {      // DOC: sleeplock0
        acquire(&ptable.lock); // DOC: sleeplock1
        release(&curLock->myLock);
    }
    
    
    // Go to sleep.
    // proc->chan is a list to store waiting processes(threads)
    // we can simply set up mutexid to conditional veriable to store waiting processes(threads)
    
    curProc->cv  = muxid;
    curProc->state = SLEEPING;
    
    sched();
    
    // Tidy up.
    // I am not sure about why we are doing this...
    // but xv6 is doing this, so I just...
    
    curProc->cv = 0;
    
    // Reacquire original lock.
    if (&curLock->myLock != &ptable.lock) { // DOC: sleeplock2
        release(&ptable.lock);
        acquire(&curLock->myLock);
    }
    
}

// for waking up process(thread)
// copy from wakeup1() and wakeup() form proc.c


void
cv_signal(int muxid)
{
    struct mutex_lock *curLock;
    for (curLock = my_lock_table.lock_List; curLock < &my_lock_table.lock_List[16]; curLock++) {
        if (curLock->lockID == muxid) {
            break;              // we have found the lock in my_lock_table
        }
    }
    
    struct proc *curProc = myproc();
    int i;
    for (i = 0; i < curProc->lockNums; i++) {
        if (curProc->mutexLocks[i] == muxid) {
            break;
        }
    }
    
    acquire(&ptable.lock);
    
    for (curProc = ptable.proc; curProc < &ptable.proc[NPROC]; curProc++)
        if (curProc->state == SLEEPING && curProc->cv == muxid)
        {
            curProc->state = RUNNABLE;

        }
    
    release(&ptable.lock);
}
*/

