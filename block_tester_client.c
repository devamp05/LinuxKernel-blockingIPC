/***************
 * Tianze Kuang
 * wde364
 * 11352826
 *
 * Devam Punitbhai Patel
 * dns682
 * 11316715
 * *************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/kernel.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

const int MaxSleep = 5;
pid_t to;
int NumArgs;
char** Args;
void TerminateLoop() { exit(0); }

void ChildProc()
{
    int original_request;
    unsigned int mesg_size;
    unsigned int reply_size;
    int reply;
    int x           = 2;
    const uint Send = 441;
    int sleepTime;
    srand(getpid());
    printf("childproc called\n");
    /* skip the first 2 parameters (process name and target pid) */
    for (int i = 0; i < 5; i++)
    {
        printf("running\n");

        for (; x < NumArgs; x++)
        {
            original_request = atoi(Args[x]);
            mesg_size        = (unsigned int) sizeof(unsigned int);
            reply_size       = (unsigned int) sizeof(unsigned int);
            printf("%d: requesting fib(%d) from %d\n",
                   getpid(),
                   original_request,
                   to);
            if (syscall(Send,
                        to,
                        (void*) &original_request,
                        mesg_size,
                        &reply,
                        &reply_size) != -1)
            {

                printf("%d: got reply for fib(%d) = %d\n",
                       getpid(),
                       original_request,
                       reply);
            }
            else
            {
                printf("There was a problem.\n");
                perror("pSend");
            }
            sleepTime = (rand() % MaxSleep);
            printf("sleep: %d\n", sleepTime);
            sleep(sleepTime);
        }
        x = 2;
    }
}

int main(int argc, char* argv[])
{
    int i = 0;
    int child;
    /* arguments: argv[1] = pid to send to
     * argv[2] - argv[n] = requests to make
     */
    if (argc < 3)
    {
        printf("usage: %s [pid] [requests] -- This app will send requests to"
               "the specified PID\n"
               "and request that the PROCESSt calculate the fib value for that"
               "value\n",
               argv[0]);
        exit(1);
    }

    to = atoi(argv[1]);
    if (to == 0)
    {
        printf("make sure your pid is a valid number\n");
    }
    NumArgs = argc;
    Args    = argv;

    signal(SIGINT, TerminateLoop);
    for (i = 0; i < 5; i++)
    {
        child = fork();
        if (child == 0)
        {
            printf("spawn %d\n", i);

            ChildProc();
            exit(0);
        }
        else if (child < 0)
        {
            puts("child spawn failed");
            continue;
        }
    }

    /* wait for all */
    while (wait(NULL) > 0) {}

    return 0;
}
