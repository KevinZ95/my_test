// Mutual exclusion lock.
struct spinlock {
	uint locked; // Is the lock held?

	// For debugging:
	char *      name;    // Name of lock.
	struct cpu *cpu;     // The cpu holding the lock.
	uint        pcs[10]; // The call stack (an array of program counters)
	                     // that locked the lock.
};


// my mutex lock
// basicly mutex_lock is spinlock
// just added some fields to make it easy to use

struct mutex_lock {
    struct spinlock     myLock;
    
    uint    locked;         // 0 - unlocked, 1 - locked
    char    name[100];      // lock name
    int     owner;      // lock owner
    int     lockID;         // lock id
    int     status;        // 0 - unused, 1 - using
};
