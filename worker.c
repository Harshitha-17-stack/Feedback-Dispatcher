#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("Worker started. PID: %d\n", getpid());
    fflush(stdout);

    while (1)
    {
        pause();
    }

    return 0;
}
