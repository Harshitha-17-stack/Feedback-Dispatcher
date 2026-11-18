#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define QUEUES 3
#define QUANTUM 1

typedef struct Process
{
    int arrival_time;
    int priority;
    int cpu_time;
    int remaining_cpu_time;
    int memory;
    int resources[4];
    int queue_level;
    pid_t pid;
    int started;
    struct Process *next;
} Process;

typedef struct
{
    Process *front;
    Process *rear;
} Queue;

Queue input_queue;
Queue feedback[QUEUES];

int dispatcher_timer = 0;
Process *running_process = NULL;

void initializeQueue(Queue *q)
{
    q->front = NULL;
    q->rear = NULL;
}

int isEmpty(Queue *q)
{
    return q->front == NULL;
}

void enqueue(Queue *q, Process *p)
{
    p->next = NULL;

    if (q->rear == NULL)
    {
        q->front = p;
        q->rear = p;
    }
    else
    {
        q->rear->next = p;
        q->rear = p;
    }
}

Process *dequeue(Queue *q)
{
    if (isEmpty(q))
        return NULL;

    Process *p = q->front;
    q->front = p->next;

    if (q->front == NULL)
        q->rear = NULL;

    p->next = NULL;
    return p;
}

int feedbackQueuesNotEmpty()
{
    for (int i = 0; i < QUEUES; i++)
    {
        if (!isEmpty(&feedback[i]))
            return 1;
    }

    return 0;
}

void loadProcesses(char *filename)
{
    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        perror("Cannot open feedback.txt");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int previous_arrival = -1;
    int count = 0;

    while (fgets(line, sizeof(line), file))
    {
        Process *p = malloc(sizeof(Process));

        if (p == NULL)
        {
            perror("Memory allocation failed");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        int result = sscanf(line,
            " %d , %d , %d , %d , %d , %d , %d , %d",
            &p->arrival_time,
            &p->priority,
            &p->cpu_time,
            &p->memory,
            &p->resources[0],
            &p->resources[1],
            &p->resources[2],
            &p->resources[3]);

        if (result != 8 ||
            p->arrival_time < 0 ||
            p->cpu_time <= 0 ||
            p->arrival_time < previous_arrival)
        {
            printf("Invalid input or arrival order: %s", line);
            free(p);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        previous_arrival = p->arrival_time;
        p->remaining_cpu_time = p->cpu_time;
        p->queue_level = 0;
        p->pid = -1;
        p->started = 0;
        p->next = NULL;

        enqueue(&input_queue, p);
        count++;
    }

    fclose(file);

    if (count == 0)
    {
        printf("No processes found in feedback.txt\n");
        exit(EXIT_FAILURE);
    }

    printf("All %d processes loaded successfully!\n", count);
}

void startProcess(Process *p)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        execl("./worker", "worker", (char *)NULL);
        perror("exec failed");
        _exit(EXIT_FAILURE);
    }

    p->pid = pid;
    p->started = 1;

    printf("Started process PID: %d\n", p->pid);
}

int suspendProcess(Process *p)
{
    if (kill(p->pid, SIGTSTP) == -1)
    {
        perror("SIGTSTP failed");
        return 0;
    }

    int status;
    pid_t result;

    do
    {
        result = waitpid(p->pid, &status, WUNTRACED);
    }
    while (result == -1 && errno == EINTR);

    if (result == -1)
    {
        perror("waitpid failed");
        return 0;
    }

    if (WIFSTOPPED(status))
    {
        printf("Suspended process PID: %d\n", p->pid);
        return 1;
    }

    printf("Process PID %d exited unexpectedly\n", p->pid);
    return 0;
}

void resumeProcess(Process *p)
{
    if (kill(p->pid, SIGCONT) == -1)
    {
        perror("SIGCONT failed");
        exit(EXIT_FAILURE);
    }

    printf("Resumed process PID: %d\n", p->pid);
}

void terminateProcess(Process *p)
{
    if (p->pid > 0)
    {
        if (kill(p->pid, SIGINT) == -1 && errno != ESRCH)
        {
            perror("SIGINT failed");
        }

        int status;
        pid_t result;

        do
        {
            result = waitpid(p->pid, &status, 0);
        }
        while (result == -1 && errno == EINTR);

        printf("Terminated process PID: %d\n", p->pid);
    }

    free(p);
}

void dispatcher()
{
    while (!isEmpty(&input_queue) ||
           feedbackQueuesNotEmpty() ||
           running_process != NULL)
    {
        while (!isEmpty(&input_queue) &&
               input_queue.front->arrival_time <= dispatcher_timer)
        {
            Process *p = dequeue(&input_queue);

            p->queue_level = 0;

            enqueue(&feedback[0], p);

            printf("Time %d: Process arrived in RQ0\n",
                   dispatcher_timer);
        }

        if (running_process != NULL)
        {
            running_process->remaining_cpu_time--;

            printf("Time %d: PID %d, Remaining CPU: %d, RQ%d\n",
                   dispatcher_timer,
                   running_process->pid,
                   running_process->remaining_cpu_time,
                   running_process->queue_level);

            if (running_process->remaining_cpu_time <= 0)
            {
                terminateProcess(running_process);
                running_process = NULL;
            }
            else if (feedbackQueuesNotEmpty())
            {
                if (suspendProcess(running_process))
                {
                    if (running_process->queue_level < QUEUES - 1)
                    {
                        running_process->queue_level++;
                    }

                    printf("Moved process to RQ%d\n",
                           running_process->queue_level);

                    enqueue(&feedback[running_process->queue_level],
                            running_process);
                }
                else
                {
                    free(running_process);
                }

                running_process = NULL;
            }
        }

        if (running_process == NULL && feedbackQueuesNotEmpty())
        {
            int i;

            for (i = 0; i < QUEUES; i++)
            {
                if (!isEmpty(&feedback[i]))
                    break;
            }

            running_process = dequeue(&feedback[i]);

            printf("Selected process from RQ%d\n", i);

            if (running_process->started)
            {
                resumeProcess(running_process);
            }
            else
            {
                startProcess(running_process);
            }
        }

        sleep(QUANTUM);
        dispatcher_timer++;
    }

    printf("\nAll processes completed!\n");
}



int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <dispatch-list-file>\n", argv[0]);
        return 1;
    }

    initializeQueue(&input_queue);

    for (int i = 0; i < QUEUES; i++)
    {
        initializeQueue(&feedback[i]);
    }

    loadProcesses(argv[1]);

    printf("\nStarting Feedback Dispatcher\n\n");

    dispatcher();

    return 0;
}
