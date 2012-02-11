/*
 * stopwatch - Kernel module for measuring execution time of specific code
 * inside Kernel space
 * Licensed under GPL v2.
 * Peter Senna Tschudin <peter.senna@gmail.com>
 *
*/

#include <linux/module.h>
#include <linux/slab.h>	/* kmalloc(), free()*/
#include <linux/time.h> /* timespec */
#include <media/videobuf2-core.h> /* data structures used in this example */

MODULE_LICENSE("GPL");

#define VIDEO_MAX_FRAME		32

void stopwatch (char msg[32], int repeats, int (*function)( struct timespec *ts_start, struct timespec *ts_end));
static int stopwatch_init(void);
int ORIGINAL (struct timespec *ts_start, struct timespec *ts_end);
int PROPOSED (struct timespec *ts_start, struct timespec *ts_end);
static void stopwatch_exit(void);

/*
 * stopwatch
 * @msg			- Message that is printed before first result
 * @repeat count	- How many times the test will be repeated?
 * @pointer to function	- Pointer to a function that returns int and receives \
 *			  two struct timespec as parameters
 *
*/
void stopwatch (char msg[32], int repeats, int (*function)( struct timespec *, struct timespec *))
{
	struct timespec begin, end, diff;
	int i = 0;

	printk ("%s", msg);

	for (i = repeats; i > 0; i--){

		if (function( &begin, &end ) != 0)
			printk("Error");

		diff = timespec_sub(end, begin);

		printk ("%lu,", diff.tv_nsec );
	}
}


/*
 * stopwatch_init
 * returns 0 on success (always)
*/
static int stopwatch_init(void)
{
	printk ("Starting stopwatch...\n");

	stopwatch("Proposed_code:, ", 512, &PROPOSED);
	stopwatch("\nOriginal_code:, ", 512, &ORIGINAL);

	return 0;
}

/*
 * ORIGINAL
 * @ts_start	- Pointer to struct timespec containing the start time
 * @ts_end	- Pointer to struct timespec containing the end time
 *
 * returns 0 on success or 1 on fail
 *
*/
int ORIGINAL (struct timespec *ts_start, struct timespec *ts_end)
{
	unsigned int i;

	struct vb2_queue *q;

	q = kmalloc(sizeof(struct vb2_queue), __GFP_WAIT | __GFP_IO | __GFP_FS);

	q->num_buffers = VIDEO_MAX_FRAME;

	for (i = 0; i < VIDEO_MAX_FRAME; ++i)
		q->bufs[i] = (struct vb2_buffer *) kmalloc(sizeof(struct vb2_buffer), __GFP_WAIT | __GFP_IO | __GFP_FS);

	getnstimeofday (ts_start); /*stopwatch start*/

	for (i = 0; i < q->num_buffers; ++i)
		q->bufs[i]->state = VB2_BUF_STATE_DEQUEUED;

	getnstimeofday (ts_end); /*stopwatch stop*/

	for (i = 0; i < VIDEO_MAX_FRAME; ++i)
		kfree(q->bufs[i]);

	kfree(q);

/*
 * Loop for testing if the result is correct
 *
	for (i = 0; i < q->num_buffers; ++i)
		if (q->bufs[i]->state != VB2_BUF_STATE_DEQUEUED)
			return 1;
*/
	return 0;
}

/*
 * PROPOSED
 * @ts_start	- Pointer to struct timespec containing the start time
 * @ts_end	- Pointer to struct timespec containing the end time
 *
 * returns 0 on success or 1 on fail
 *
*/
int PROPOSED (struct timespec *ts_start, struct timespec *ts_end)
{
	unsigned int i;

	struct vb2_queue *q;
	struct vb2_buffer *buf_ptr, *buf_ptr_end;

	q = kmalloc(sizeof(struct vb2_queue), __GFP_WAIT | __GFP_IO | __GFP_FS);

	q->num_buffers = VIDEO_MAX_FRAME;

	for (i = 0; i < VIDEO_MAX_FRAME; ++i)
		q->bufs[i] = (struct vb2_buffer *) kmalloc(sizeof(struct vb2_buffer), __GFP_WAIT | __GFP_IO | __GFP_FS);

	getnstimeofday (ts_start); /*stopwatch start*/

	buf_ptr_end = q->bufs[q->num_buffers];

	for (buf_ptr = q->bufs[0]; buf_ptr < buf_ptr_end; ++buf_ptr)
		buf_ptr->state = VB2_BUF_STATE_DEQUEUED;

	getnstimeofday (ts_end); /*stopwatch stop*/

	for (i = 0; i < VIDEO_MAX_FRAME; ++i)
		kfree(q->bufs[i]);

	kfree(q);

/*
 * Loop for testing if the result is correct
 *

	for (i = 0; i < q->num_buffers; ++i)
		if (q->bufs[i]->state != VB2_BUF_STATE_DEQUEUED)
			return 1;
*/
	return 0;

}

/*
 * stopwatch_exit
 *
*/
static void stopwatch_exit(void)
{

	printk ("Exiting...\n");
}


module_init(stopwatch_init);
module_exit(stopwatch_exit);
