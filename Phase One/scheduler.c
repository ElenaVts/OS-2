#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 80

void fcfs(int numprocs);
void rr();

#define PROC_NEW	0
#define PROC_STOPPED	1
#define PROC_RUNNING	2
#define PROC_EXITED	3

typedef struct proc_desc {
	struct proc_desc *next;
	char name[80];
	int pid;
	int status;
	int num_procs; /////////////////////////////////////////////////////////////
	double t_submission, t_start, t_end;
} proc_t;

struct single_queue {
	proc_t	*first;
	proc_t	*last;
	long members;
};

struct single_queue global_q;

#define proc_queue_empty(q) ((q)->first==NULL)

void proc_queue_init (register struct single_queue * q)
{
	q->first = q->last = NULL;
	q->members = 0;
}

void proc_to_rq (register proc_t *proc)
{
	if (proc_queue_empty (&global_q))
		global_q.last = proc;
	proc->next = global_q.first;
	global_q.first = proc;
}

void proc_to_rq_end (register proc_t *proc)
{
	if (proc_queue_empty (&global_q))
		global_q.first = global_q.last = proc;
	else {
		global_q.last->next = proc;
		global_q.last = proc;
		proc->next = NULL;
	}
}

proc_t *proc_rq_dequeue ()
{
	register proc_t *proc;

	proc = global_q.first;
	if (proc==NULL) return NULL;

	proc = global_q.first;
	if (proc!=NULL) {
		global_q.first = proc->next;
		proc->next = NULL;
	}

	return proc;
}


void print_queue()
{
	proc_t *proc;

	proc = global_q.first;
	while (proc != NULL) {
		printf("proc: [name:%s pid:%d]\n", 
			proc->name, proc->pid);
		proc = proc->next;
	}
}

double proc_gettime()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (double) (tv.tv_sec+tv.tv_usec/1000000.0);
}

#define FCFS	0
#define RR	1

int policy = FCFS;
int quantum = 100;	/* ms */
proc_t *running_proc;
double global_t;

void err_exit(char *msg)
{
	printf("Error: %s\n", msg);
	exit(1);
}

int main(int argc,char **argv)
{
	FILE *input;
	char exec[80];
	int c, num_processors;
	int numprocs;
	proc_t *proc;

	if (argc == 1) {
		err_exit("invalid usage");
	} else if (argc == 2) {
		input = fopen(argv[1],"r");
		if (input == NULL) err_exit("invalid input file name");
		num_processors = 1;
	} else if (argc > 2) {
		if (!strcmp(argv[1],"FCFS")) {
			policy = FCFS;
			input = fopen(argv[2],"r");
			if (input == NULL) err_exit("invalid input file name");
			if (argc > 3) {
                num_processors = atoi(argv[3]);
            } else {
                num_processors = 1; // Default to 1 processor
            }
		} else if (!strcmp(argv[1],"RR")) {
			policy = RR;
			quantum = atoi(argv[2]);
			input = fopen(argv[3],"r");
			if (input == NULL) err_exit("invalid input file name");
			if (argc > 4) {
                num_processors = atoi(argv[4]);
            } else {
                num_processors = 1; // Default to 1 processor
            }
		} else {
			err_exit("invalid usage");
		}
	}
    proc_queue_init(&global_q);
    
	/* Read input file */
	while ((c=fscanf(input, "%s %d", exec, &numprocs))!=EOF) { ///////////////////////////////
		// printf("fscanf returned %d\n", c);
		// printf("exec = %s\n", exec);

		proc = malloc(sizeof(proc_t));
		proc->next = NULL;
		strcpy(proc->name, exec);
		proc->pid = -1;
		proc->status = PROC_NEW;
		proc->num_procs = numprocs; /////////////////////////////////////////////
		proc->t_submission = proc_gettime();
		proc_to_rq_end(proc);
	}
    fclose(input);
    
	//print_queue(&global_q);

  	global_t = proc_gettime();
	switch (policy) {
		case FCFS:
			fcfs(num_processors);
			break;

		case RR:
			rr();
			break;

		default:
			err_exit("Unimplemented policy");
			break;
	}

	printf("WORKLOAD TIME: %.2lf secs\n", proc_gettime()-global_t);
	printf("scheduler exits\n");
	return 0;
}


/*void fcfs(int numprocs)
{
    proc_t *processor_queues[numprocs];
	proc_t *proc;
	int pid;
	int status;

    for (int i = 0; i < numprocs; i++) {
        processor_queues[i] = NULL;
    }
    
    int count = 0; // Count total processes
    proc_t *temp_proc = global_q.first;
    while (temp_proc != NULL) {
        count++;
        temp_proc = temp_proc->next;
    }
    
    proc_t *processes[count];
    temp_proc = global_q.first;
    for (int i = 0; i < count; i++) {
        processes[i] = temp_proc;
        temp_proc = temp_proc->next;
    }
    
    
    
	while ((proc=proc_rq_dequeue()) != NULL) {
		// printf("Dequeued process with name %s\n", proc->name);
		if (proc->status == PROC_NEW) {
			proc->t_start = proc_gettime();
			pid = fork();
			if (pid == -1) {
				err_exit("fork failed!");
			}
			if (pid == 0) { /////////////////////////////anathesi ergasias se epejergasti.
				printf("executing %s\n", proc->name);
				execl(proc->name, proc->name, NULL);
			} else {
				proc->pid = pid;
				proc->status = PROC_RUNNING;
				pid = waitpid(proc->pid, &status, 0);
				proc->status = PROC_EXITED;
				if (pid < 0) err_exit("waitpid failed");
				proc->t_end = proc_gettime();
				printf("PID %d - CMD: %s\n", pid, proc->name);
				printf("\tElapsed time = %.2lf secs\n", proc->t_end-proc->t_submission);
				printf("\tExecution time = %.2lf secs\n", proc->t_end-proc->t_start);
				printf("\tWorkload time = %.2lf secs\n", proc->t_end-global_t);
				//////////////////////////////////////////////////////eleutherwsi epejergasti.
			}
		}
	}
}*/

void fcfs(int numprocs) {
    proc_t *proc;
    int *processor_status;
    proc_t **processors;
    int i, status;
    pid_t pid;

    // Allocate memory for processors and processor statuses dynamically
    processors = malloc(numprocs * sizeof(proc_t *));
    processor_status = malloc(numprocs * sizeof(int));
    if (!processors || !processor_status) {
        err_exit("Memory allocation failed for processors or statuses");
    }

    // Initialize all processors as available
    for (i = 0; i < numprocs; i++) {
        processors[i] = NULL;
        processor_status[i] = PROC_NEW;
    }

    while ((proc = proc_rq_dequeue()) != NULL) {
        // Find the next available processor
        for (i = 0; i < numprocs; i++) {
            if (processor_status[i] == PROC_EXITED || processor_status[i] == PROC_NEW) {
                break; // Found an available processor
            }
        }

        if (i == numprocs) {
            // No processors are free; wait for one to complete
            pid = waitpid(-1, &status, 0);
            if (pid < 0) err_exit("waitpid failed");
            for (i = 0; i < numprocs; i++) {
                if (processors[i] && processors[i]->pid == pid) {
                    processors[i]->status = PROC_EXITED;
                    processors[i]->t_end = proc_gettime();
                    printf("Processor %d - PID %d - CMD: %s\n", i + 1, pid, processors[i]->name);
                    printf("\tElapsed time = %.2lf secs\n", processors[i]->t_end - processors[i]->t_submission);
                    printf("\tExecution time = %.2lf secs\n", processors[i]->t_end - processors[i]->t_start);
                    printf("\tWorkload time = %.2lf secs\n", processors[i]->t_end - global_t);
                    break;
                }
            }
        }

        // Assign the dequeued process to the processor
        processors[i] = proc;
        proc->t_start = proc_gettime();
        pid = fork();

        if (pid == -1) {
            err_exit("fork failed!");
        } else if (pid == 0) {
            printf("Processor %d executing %s\n", i + 1, proc->name);
            execl(proc->name, proc->name, NULL);
        } else {
            proc->pid = pid;
            proc->status = PROC_RUNNING;
            processor_status[i] = PROC_RUNNING;
        }
    }

    // Wait for remaining processes to finish
    for (i = 0; i < numprocs; i++) {
        if (processor_status[i] == PROC_RUNNING) {
            pid = waitpid(processors[i]->pid, &status, 0);
            if (pid < 0) err_exit("waitpid failed");
            processors[i]->status = PROC_EXITED;
            processors[i]->t_end = proc_gettime();
            printf("Processor %d - PID %d - CMD: %s\n", i + 1, pid, processors[i]->name);
            printf("\tElapsed time = %.2lf secs\n", processors[i]->t_end - processors[i]->t_submission);
            printf("\tExecution time = %.2lf secs\n", processors[i]->t_end - processors[i]->t_start);
            printf("\tWorkload time = %.2lf secs\n", processors[i]->t_end - global_t);
        }
    }

    // Free dynamically allocated memory
    free(processors);
    free(processor_status);
}




void sigchld_handler(int signo, siginfo_t *info, void *context)
{
	printf("child %d exited\n", info->si_pid);
	if (running_proc == NULL) {
		printf("warning: running_proc==NULL\n");
	} else if (running_proc->pid == info->si_pid) {
		running_proc->status = PROC_EXITED;
		proc_t *proc = running_proc;
		proc->t_end = proc_gettime();
		printf("PID %d - CMD: %s\n", proc->pid, proc->name);
		printf("\tElapsed time = %.2lf secs\n", proc->t_end-proc->t_submission);
		printf("\tExecution time = %.2lf secs\n", proc->t_end-proc->t_start);
		printf("\tWorkload time = %.2lf secs\n", proc->t_end-global_t);

	} else {
		printf("warning: running %d exited %d\n", running_proc->pid, info->si_pid);
	}
}

void rr()
{
	struct sigaction sig_act;
	proc_t *proc;
	int pid;
	struct timespec req, rem;

	req.tv_sec = quantum / 1000;
	req.tv_nsec = (quantum % 1000)*1000000;

	printf("tv_sec = %ld\n", req.tv_sec);
	printf("tv_nsec = %ld\n", req.tv_nsec);

	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_handler = 0;
	sig_act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
	sig_act.sa_sigaction = sigchld_handler;
	sigaction (SIGCHLD,&sig_act,NULL);

	while ((proc=proc_rq_dequeue()) != NULL) {
		// printf("Dequeue process with name %s and pid %d\n", proc->name, proc->pid);
		if (proc->status == PROC_NEW) {
			proc->t_start = proc_gettime();
			pid = fork();
			if (pid == -1) {
				err_exit("fork failed!");
			}
			if (pid == 0) {
				printf("executing %s\n", proc->name);
				execl(proc->name, proc->name, NULL);
			}
			else {
				proc->pid = pid;
				running_proc = proc;
				proc->status = PROC_RUNNING;

				nanosleep(&req, &rem);
				if (proc->status == PROC_RUNNING) {
					kill(proc->pid, SIGSTOP);
					proc->status = PROC_STOPPED;
					proc_to_rq_end(proc);
				}
				else if (proc->status == PROC_EXITED) {
				}

			}
		}
		else if (proc->status == PROC_STOPPED) {
			proc->status = PROC_RUNNING;
			running_proc = proc;
			kill(proc->pid, SIGCONT);

			nanosleep(&req, &rem);
			if (proc->status == PROC_RUNNING) {
				kill(proc->pid, SIGSTOP);
				proc_to_rq_end(proc);
				proc->status = PROC_STOPPED;
			}
			else if (proc->status == PROC_EXITED) {
			}

		}
		else if (proc->status == PROC_EXITED) {
			printf("process has exited\n");
		}
		else if (proc->status == PROC_RUNNING) {
			printf("WARNING: Already running process\n");
		}
		else {
			err_exit("Unknown process status");
		}
	}
}
