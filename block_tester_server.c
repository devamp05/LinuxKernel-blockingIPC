/***************
 * Tianze Kuang
 * wde364
 * 11352826
 *
 * Devam Punitbhai Patel
 * dns682
 * 11316715
 * *************/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/kernel.h>
#include <unistd.h>

int fib(int N)
{
    if (N == 0)
        return 1;
    if (N == 1)
        return 1;

    return fib(N - 1) + fib(N - 2);
}

const int SleepTime = 10;

int main(int argc, char* argv[])
{
    pid_t from = 0;
    int request;
    int request_size = sizeof(int);
    int reply;
    const uint Receive = 442, Reply = 443, MsgWaits = 444;

    printf("Here's your pid, stupid: %d\n", getpid());

	while(syscall(MsgWaits) == 0){printf("no msg yet, check in 2 secs\n");
	sleep(2);}

	printf("There is msg! start working\n");

    while (1)
    {
        printf("from: %x \t request: %d \t request_size: %d \n",
               from,
               request,
               request_size);
        request_size = sizeof(request);
        if (syscall(Receive, &from, &request, &request_size) != -1)
        {
            printf("%d: got request for fib(%d) from %d\n",
                   getpid(),
                   request,
                   from);
            reply = fib(request);
            printf("%d: sending reply fib(%d) = %d to %d\n",
                   getpid(),
                   request,
                   reply,
                   from);
            syscall(Reply, from, &reply, sizeof(reply));
        }
        else
        {
            printf("Error with receive request at server.\n");
        }
        sleep(rand() % SleepTime);
    }
}
