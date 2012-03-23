/*
 * yfkm2 - Your first Kernel Modification v2
 * Peter Senna Tschudin <peter.senna@gmail.com>
 *
 */

/*
 * KNOWN BUGS:
 * Does not work when trying to notify more than one process for same monitored
 * PID.
 *
 */

/*
 * TODO:
 *
 * Split .c in .c + .h
 *
 * Check if Kernel thread started correctly and treat possible errors
 *
 * Check if yfkm2_list->notify != 0 before seting new value
 *
 */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>

#define YFKM2_KT_TIMEOUT (1*HZ) /* 1 second */

struct yfkm2 {
	pid_t monitor;		/* PID to monitor */
	pid_t notifyme;		/* PID to notify */
	struct list_head list;	/* Linked List struct */
};

/* How many Kernel Threads are running? */
atomic_t yfkm2_kthread_run_count = ATOMIC_INIT(0);

/* Define and initialize yfkm2_(linked)list */
LIST_HEAD(yfkm2_list);

/* Define and initialize yfkm2_(read&write)lock */
DEFINE_RWLOCK(yfkm2_lock);

/*
 * yfkm2_is_pid_running(pid_t pid)
 *
 * Check if pid is running
 *
 * return 0 if pid is running
 * return 1 if pid is not running
 */
int yfkm2_is_pid_running(pid_t pid)
{
	struct task_struct *q;

	rcu_read_lock();
	q = find_task_by_vpid(pid);
	rcu_read_unlock();

	if (q != NULL && q->pid == pid)
		return 0;
	else
		return 1;
}

/*
 * yfkm2_kill(pid_t pid)
 *
 * Kills pid
 *
 * return 0 if pid was running and send SIGKILL to pid
 * return 1 if pid is not running
 */
int yfkm2_kill(pid_t pid)
{
	struct task_struct *q;
	int ret;

	rcu_read_lock();
	q = find_task_by_vpid(pid);
	rcu_read_unlock();

	if (q != NULL) {
		force_sig(SIGKILL, q);
		return 0;
	}

	return 1;
}

/*
 * int yfkm2_kthread(void *data)
 *
 * The Kernel Thread
 *
 * Traverse the yfkm2_list looking for yfkm2->notifyme that are not 0.
 * If any found, check if correspondent yfkm2->monitor is still running. If not
 * kill yfkm2->notifyme. After traversing the list, check if the list is empty.
 * If so return 0. If not sleep one second and start again.
 *
 * return 0 if yfkm2_list is empty
 * should never return 1
 */
int yfkm2_kthread(void *data) /* data is NEVER used */
{
	struct yfkm2 *yfkm2_tmp, *yfkm2_tmp2;
	bool empty;

	while (true) {
		/* Needs write protection due possible item removal from list */
		write_lock(&yfkm2_lock); /* Write lock */
		list_for_each_entry_safe(yfkm2_tmp, yfkm2_tmp2,
						&yfkm2_list, list) {
			if (yfkm2_tmp->notifyme != 0) {
				if (yfkm2_is_pid_running(yfkm2_tmp->monitor) != 0) {
					yfkm2_kill(yfkm2_tmp->notifyme);
					list_del(&yfkm2_tmp->list);
					kfree(yfkm2_tmp);
				}
			}
		}
		write_unlock(&yfkm2_lock); /* Write unlock */

		read_lock(&yfkm2_lock); /* Read lock */
		empty = list_empty(&yfkm2_list);
		read_unlock(&yfkm2_lock); /* Read unlock */

		if (empty) {
			/* The counter is increased at sys_yfkm2_notifyme()
			 * Before exit, decrease atomic run counter */
			atomic_dec(&yfkm2_kthread_run_count);
			return 0;
		}

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(YFKM2_KT_TIMEOUT);
	}
	/* Before exit, decrease atomic run counter */
	atomic_dec(&yfkm2_kthread_run_count);
	return 1;
}

/*
 * asmlinkage long sys_yfkm2_monitor(pid_t monitor)
 *
 * The system call that check if monitor correspond to a running pid and stores
 * monitor at yfkm2_list->monitor
 *
 * return 0 if pid is running
 * return 1 if pid is not running
 */
asmlinkage long sys_yfkm2_monitor(pid_t monitor)
{
	struct yfkm2 *yfkm2_tmp;

	if (yfkm2_is_pid_running(monitor) == 0) {

		yfkm2_tmp = kmalloc(sizeof(*yfkm2_tmp), GFP_KERNEL);
		yfkm2_tmp->monitor = monitor;
		yfkm2_tmp->notifyme = 0;

		write_lock(&yfkm2_lock);
		list_add(&yfkm2_tmp->list, &yfkm2_list);
		write_unlock(&yfkm2_lock);

		return 0;
	}


	return 1;
}

/*
 * asmlinkage long sys_yfkm2_notifyme(pid_t monitor, pid_t notifyme)
 *
 * The system call that looks for monitor at yfkm2_list->monitor. If found
 * store notifyme at yfkm2_list->notifyme. It also starts the kernel thread
 * if it is not running.
 *
 * return 0 if pid is running
 * return 1 if pid is not running
 */
asmlinkage long sys_yfkm2_notifyme(pid_t monitor, pid_t notifyme)
{
	struct yfkm2 *yfkm2_tmp;
	bool found_monitored_pid = false;

	write_lock(&yfkm2_lock); /* Write lock */
	list_for_each_entry(yfkm2_tmp, &yfkm2_list, list) {
		if (yfkm2_tmp->monitor == monitor) {
			yfkm2_tmp->notifyme = notifyme;

			found_monitored_pid = true;

			break;
		}
	}
	write_unlock(&yfkm2_lock); /* Write unlock */

	if (found_monitored_pid) {
		if (atomic_read(&yfkm2_kthread_run_count) < 1) {
			/* The counter is decreased at yfkm2_kthread()
			 * Before start, increase atomic run counter */
			atomic_inc(&yfkm2_kthread_run_count);
			kthread_run(&yfkm2_kthread, NULL, "yfkm2_kthread");
		}

		return 0;
	} else
		return 1;
}
