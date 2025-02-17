/***************
 * Tianze Kuang
 * wde364
 * 11352826
 *
 * Devam Punitbhai Patel
 * dns682
 * 11316715
 * *************/

#ifndef __LINUX_BLOCK_SEND_H
#define __LINUX_BLOCK_SEND_H

#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

struct messageInfo
{
    pid_t* buf;
    unsigned int front;
    unsigned int back;
    unsigned int size;

    spinlock_t mutex;
    /* destructor wait on this mutex */
    spinlock_t destroy_mutex;

    /*parameters for data transfer*/
    void* sendmsg;
    unsigned int sendlen;
    void* receivemsg;
    unsigned int receivelen;
};
#endif
