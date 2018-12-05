#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
	return fork();
}

int
sys_exit(void)
{
	exit();
	return 0; // not reached
}

int
sys_wait(void)
{
	return wait();
}

int
sys_kill(void)
{
	int pid;

	if (argint(0, &pid) < 0) return -1;
	return kill(pid);
}

int
sys_getpid(void)
{
	return myproc()->pid;
}

int
sys_sbrk(void)
{
	int addr;
	int n;

	if (argint(0, &n) < 0) return -1;
	addr = myproc()->sz;
	if (growproc(n) < 0) return -1;
	return addr;
}

int
sys_sleep(void)
{
	int  n;
	uint ticks0;

	if (argint(0, &n) < 0) return -1;
	acquire(&tickslock);
	ticks0 = ticks;
	while (ticks - ticks0 < n) {
		if (myproc()->killed) {
			release(&tickslock);
			return -1;
		}
		sleep(&ticks, &tickslock);
	}
	release(&tickslock);
	return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
	uint xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}

int
sys_mux_create(void)
{

    char *curLock;     // a char pointer
    
    if (argptr(0, (void *)&curLock, sizeof(curLock[0])) < 0) return -1;
    
    int result = mutex_create(curLock);
    
    return result;
}


int
sys_mux_delete(void)
{
    int curLock;
    if (argint(0, &curLock) < 0) return -1;
    mutex_delete(curLock);
    return 0;
}


int
sys_mux_lock(void)
{
    int curLock;
    if (argint(0, &curLock) < 0) return -1;
    mutex_lock(curLock);
    return 0;
}

int
sys_mux_unlock(void)
{
    int curLock;
    if (argint(0, &curLock) < 0) return -1;
    mutex_unlock(curLock);
    return 0;
}
/*
int
sys_muxcv_wait(void)
{
    int curLock;
    if (argint(0, &curLock) < 0) return -1;
    cv_wait(curLock);
    return 0;
}

int
sys_muxcv_signal(void)
{
    int curLock;
    if (argint(0, &curLock) < 0) return -1;
    cv_signal(curLock);
    return 0;
    
}
*/ 
