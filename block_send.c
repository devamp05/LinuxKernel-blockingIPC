/***************
 * Tianze Kuang
 * wde364
 * 11352826
 *
 * Devam Punitbhai Patel
 * dns682
 * 11316715
 * *************/

#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/block_send.h>
#include <linux/sched.h>

/* 0 on success */
int insertToSendersQueue(pid_t p, struct messageInfo* minfo)
{
    if (minfo->size == 128)
    {
        return -1;
    }
    minfo->buf[minfo->back] = p;
    minfo->back++;
    minfo->size++;
    if (minfo->back == 127)
    {
        minfo->back = 0;
    }
    return 0;
}

/* returns 0 on failure */
pid_t getFromSendersQueue(struct messageInfo* minfo)
{
    pid_t result;
    if (minfo->size == 0)
    {
        return 0;
    }
    result = minfo->buf[minfo->front];
    minfo->front++;
    minfo->size--;
    if (minfo->front == 127)
    {
        minfo->front = 0;
    }
    return result;
}

SYSCALL_DEFINE5(pSend,
                pid_t,
                to,
                const void __user*,
                sData,
                unsigned int,
                slen,
                void __user*,
                rData,
                unsigned int __user*,
                rlen)
{
    unsigned int krlen;
    struct task_struct* self  = current;
    /*get to's task_struct*/
    struct task_struct* toPtr = find_task_by_vpid(to);

    unsigned long flags;

    if (!toPtr)
    {
        printk(KERN_ERR "send: target doesnt exist\n");
        return -EFAULT;
    }

    if (copy_from_user(&krlen, rlen, sizeof(unsigned int)))
    {
        printk(KERN_ERR "send failed to get len from user\n");
        /*return error on not being able to copy data from user like in example
         * in linux kernel development book*/
        return -EFAULT;
    }

    if ((slen > 0 && !sData) || (krlen > 0 && !rData))
    {
        /*error*/
        printk(KERN_ERR "send: length mismatch\n");
        return -EFAULT;
    }

    /*now first put data from user space to current's messageInfo*/
    self->minfo.sendmsg = kmalloc(slen, GFP_KERNEL);
    if (!self->minfo.sendmsg)
    {
        printk(KERN_ERR "send msg buffer alloc failed\n");
        return -EFAULT;
    }

    if (copy_from_user(self->minfo.sendmsg, sData, slen))
    {
        printk(KERN_ERR "send: copy msg failed\n");
        kfree(self->minfo.sendmsg);
        self->minfo.sendmsg = 0;
        return -EFAULT;
    }


    self->minfo.sendlen = slen;

    /*set how much sender expects to receive*/
    self->minfo.receivelen = krlen;


    /*turn off interrupts */
    spin_lock_irqsave(&toPtr->minfo.mutex, flags);

    if (insertToSendersQueue(self->pid, &toPtr->minfo) != 0)
    {
        pr_err("send: message queue full try again\n");
        spin_unlock_irqrestore(&toPtr->minfo.mutex, flags);
        return -EFAULT;
    }
    pr_info("putting sender pid : %d\n", self->pid);

    spin_unlock_irqrestore(&toPtr->minfo.mutex, flags);

    wake_up_process(toPtr);

    set_current_state(TASK_INTERRUPTIBLE);
    schedule();

    /* if we ctrl-c before response we exit */
    if (self->minfo.receivemsg)
    {
        /*once woken up copy the reply message back to user space*/
        if (copy_to_user(rData, self->minfo.receivemsg, self->minfo.receivelen))
        {
            printk(KERN_ERR "send: copying reply failed\n");
            kfree(self->minfo.receivemsg);
            self->minfo.receivemsg = 0;

            return -EFAULT;
        }

        kfree(self->minfo.receivemsg);
        self->minfo.receivemsg = 0;

        if (copy_to_user(rlen, &self->minfo.receivelen, sizeof(unsigned int)))
        {
            printk(KERN_ERR "send: receive len copy failed\n");
            return -EFAULT;
        }
    }
    else
    {
        rlen = 0;
    }
    return 0;
}

SYSCALL_DEFINE3(pReceive,
                pid_t __user*,
                from,
                void __user*,
                data,
                unsigned int __user*,
                len)
{
    /*get the sender*/
    struct task_struct* sender;
    struct task_struct* self = current;
    pid_t senderPid;
    unsigned long flags;

    unsigned int klen;

    if (!from)
    {
        /*error because receiver will need senders id to reply*/
        printk(KERN_ERR "receive: from needed\n");
        return -EFAULT;
    }
    if (!data || !len)
    {
        /*error either both be null or both be not null*/
        printk(KERN_ERR "receive: len and data both should not be null\n");
        return -EFAULT;
    }


    spin_lock_irqsave(&self->minfo.mutex, flags);

    if (self->minfo.size == 0)
    {
        spin_unlock_irqrestore(&self->minfo.mutex, flags);

        pr_err("receive: fifo is empty\n");

        set_current_state(TASK_INTERRUPTIBLE);

        schedule();

        spin_lock_irqsave(&self->minfo.mutex, flags);
    }

    senderPid = getFromSendersQueue(&self->minfo);
    if (senderPid == 0)
    {
        pr_err("receive: failed to get from fifo\n");
        spin_unlock_irqrestore(&self->minfo.mutex, flags);
        return -EFAULT;
    }

    spin_unlock_irqrestore(&self->minfo.mutex, flags);

    pr_info("received sender pid : %d\n", senderPid);

    sender = find_task_by_vpid(senderPid);
    if (!sender)
    {
        printk(KERN_ERR "receive: sender not found\n");
        return -EFAULT;
    }

    spin_lock_irqsave(&sender->minfo.destroy_mutex, flags);
	
    if (copy_from_user(&klen, len, sizeof(unsigned int)))
    {
        printk(KERN_ERR "receive: getting len from use failed\n");
        kfree(sender->minfo.sendmsg);
        sender->minfo.sendmsg = 0;
        return -EFAULT;
    }


    if (klen > sender->minfo.sendlen)
    {
        klen = sender->minfo.sendlen;
    }

    if (copy_to_user(data, sender->minfo.sendmsg, klen))
    {
        printk(KERN_ERR "receive: copying message to user failed\n");
        kfree(sender->minfo.sendmsg);
        sender->minfo.sendmsg = 0;
        spin_unlock_irqrestore(&sender->minfo.destroy_mutex, flags);
        return -EFAULT;
    }

    kfree(sender->minfo.sendmsg);
    sender->minfo.sendmsg = 0;

    if (copy_to_user(from, &sender->pid, sizeof(pid_t)))
    {
        printk(KERN_ERR "receive: copying senders pid to user failed\n");
        spin_unlock_irqrestore(&self->minfo.mutex, flags);
        return -EFAULT;
    }

    spin_unlock_irqrestore(&sender->minfo.destroy_mutex, flags);

    /*set len to klen*/
    if (copy_to_user(len, &klen, sizeof(unsigned int)))
    {
        printk(KERN_ERR "receive: copying len to user failed\n");

        return -EFAULT;
    }

    return 0;
}

SYSCALL_DEFINE3(
    pReply, pid_t, senderId, const void __user*, data, unsigned int, len)
{
    struct task_struct* sender = find_task_by_vpid(senderId);
    unsigned long flags;

    if (!sender)
    {
        pr_err("reply: sender already exited\n");
        return -EFAULT;
    }

    spin_lock_irqsave(&sender->minfo.destroy_mutex, flags);
    if (len > 0 && !data)
    {
        /*error*/
        printk(KERN_ERR "reply: len can't be > 0 when data is null\n");
        spin_unlock_irqrestore(&sender->minfo.destroy_mutex, flags);
        return -EFAULT;
    }

    if (len > 0)
    {
        if (len > sender->minfo.receivelen)
        {
            len = sender->minfo.receivelen;
        }

        sender->minfo.receivemsg = kmalloc(len, GFP_KERNEL);
        if (!sender->minfo.receivemsg)
        {
            printk(KERN_ERR "reply: kmalloc for reply failed\n");
            spin_unlock_irqrestore(&sender->minfo.destroy_mutex, flags);
            return -EFAULT;
        }

        if (copy_from_user(sender->minfo.receivemsg, data, len))
        {
        	printk(KERN_ERR "reply: copying message "
			"from user failed in reply\n");
            spin_unlock_irqrestore(&sender->minfo.destroy_mutex, flags);
            return -EFAULT;
        }

        sender->minfo.receivelen = len;
    }

    wake_up_process(sender);

    spin_unlock_irqrestore(&sender->minfo.destroy_mutex, flags);

    return 0;
}

/* 1 if have message */
SYSCALL_DEFINE0(pMsgWaits)
{
    unsigned long flags;
    int amount               = 0;
    struct task_struct* self = current;

    spin_lock_irqsave(&self->minfo.mutex, flags);

    amount = self->minfo.size;

    spin_unlock_irqrestore(&self->minfo.mutex, flags);

    return amount;
}
