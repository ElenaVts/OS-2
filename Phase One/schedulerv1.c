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
void rr(int num_processors);
void rr_aff(int num_processors);

#define PROC_NEW	0
#define PROC_STOPPED	1
#define PROC_RUNNING	2
#define PROC_EXITED	3

typedef struct proc_desc {
	struct proc_desc *next;
	char name[80];
	int pid;
	int status;
	int num_procs;
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
#define RRAFF         2


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
		num_processors = 1; // Default to 1 processor
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
		} else if (!strcmp(argv[1], "RR-AFF")) {
   			policy = RRAFF; 
   		 	quantum = atoi(argv[2]);
  		  	input = fopen(argv[3], "r");
 		  	if (input == NULL) err_exit("invalid input file name");
    			if (argc > 4) {
     	     num_processors = atoi(argv[4]);
   	 } else {
       	      num_processors = 1; // Default to 1 processor
    	}

		}else {
			err_exit("invalid usage");
		}
	}
    proc_queue_init(&global_q);
    
	/* Read input file */
	while ((c=fscanf(input, "%s %d", exec, &numprocs))!=EOF) {
		// printf("fscanf returned %d\n", c);
		// printf("exec = %s\n", exec);

		proc = malloc(sizeof(proc_t));
		proc->next = NULL;
		strcpy(proc->name, exec);
		proc->pid = -1;
		proc->status = PROC_NEW;
		proc->num_procs = numprocs;
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
			rr(num_processors);
    			break;
		 
		case RRAFF: 

        			rr_aff(num_processors);
      			  break;
		default:
			err_exit("Unimplemented policy");
			break;
	}

	printf("WORKLOAD TIME: %.2lf secs\n", proc_gettime()-global_t);
	printf("scheduler exits\n");
	return 0;
}



void fcfs(int numprocs) {
    proc_t *proc;
    int *processor_status;
    proc_t **processors;
    int i, status;
    pid_t pid;
	
    processors = malloc(numprocs * sizeof(proc_t *));
    processor_status = malloc(numprocs * sizeof(int));
    if (!processors || !processor_status) {
        err_exit("Memory allocation failed for processors or statuses");
    }

    // Set all processors as available
    for (i = 0; i < numprocs; i++) {
        processors[i] = NULL;
        processor_status[i] = PROC_NEW;
    }

    while ((proc = proc_rq_dequeue()) != NULL) {
        // Find the next available processor
        for (i = 0; i < numprocs; i++) {
            if (processor_status[i] == PROC_EXITED || processor_status[i] == PROC_NEW) {
                break;
            }
        }

        if (i == numprocs) {
            // No free processors. Waiting list.
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

    // Free memory
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



void rr(int num_processors) {
    struct sigaction sig_act;
    proc_t *processors[num_processors];
    int processor_status[num_processors];
    int i, pid, status;
    struct timespec req, rem;

    // Αρχικοποίηση κβάντου χρόνου
    req.tv_sec = quantum / 1000;
    req.tv_nsec = (quantum % 1000) * 1000000;

    // Αρχικοποίηση επεξεργαστών
    for (i = 0; i < num_processors; i++) {
        processors[i] = NULL;
        processor_status[i] = PROC_NEW;
    }

    // Εγκατάσταση χειριστή για SIGCHLD
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_handler = 0;
    sig_act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
    sig_act.sa_sigaction = sigchld_handler;
    sigaction(SIGCHLD, &sig_act, NULL);

    while (1) {
        // Εύρεση ελεύθερου επεξεργαστή
        for (i = 0; i < num_processors; i++) {
            if (processors[i] == NULL || processors[i]->status == PROC_EXITED) {
                processors[i] = NULL; // Καθαρισμός ολοκληρωμένων διεργασιών
            }
        }

        // Αναζήτηση διεργασιών από την ουρά
        for (i = 0; i < num_processors; i++) {
            if (processors[i] == NULL) {
                proc_t *proc = proc_rq_dequeue();
                if (proc == NULL) continue; // Δεν υπάρχουν άλλες διεργασίες στην ουρά

                proc->t_start = proc_gettime();
                pid = fork();

                if (pid == -1) {
                    err_exit("fork failed!");
                } else if (pid == 0) {
                    execl(proc->name, proc->name, NULL);
                } else {
                    proc->pid = pid;
                    proc->status = PROC_RUNNING;
                    processors[i] = proc;
                    processor_status[i] = PROC_RUNNING;

                    printf("Processor %d executing process %s\n", i + 1, proc->name);
                }
            }
        }

        // Εκ περιτροπής έλεγχος των διεργασιών
        for (i = 0; i < num_processors; i++) {
            if (processors[i] && processors[i]->status == PROC_RUNNING) {
                nanosleep(&req, &rem); // Εκτέλεση για το κβάντο χρόνου

                if (processors[i]->status == PROC_RUNNING) {
                    kill(processors[i]->pid, SIGSTOP); // Διακοπή της διεργασίας
                    processors[i]->status = PROC_STOPPED;
                    proc_to_rq_end(processors[i]); // Επιστροφή στην ουρά
                    processors[i] = NULL;
                }
            }
        }

        // Έλεγχος για ολοκλήρωση όλων των διεργασιών
        int active_processes = 0;
        for (i = 0; i < num_processors; i++) {
            if (processors[i] != NULL) active_processes++;
        }
        if (active_processes == 0 && proc_queue_empty(&global_q)) break;
    }
}



void rr_aff(int num_processors) {
    struct sigaction sig_act;
    proc_t *processors[num_processors];
    proc_t *affinity_map[num_processors]; // Χάρτης συνάφειας
    int processor_status[num_processors];
    int i, pid, status;
    struct timespec req, rem;

    // Αρχικοποίηση κβάντου χρόνου
    req.tv_sec = quantum / 1000;
    req.tv_nsec = (quantum % 1000) * 1000000;

    // Αρχικοποίηση επεξεργαστών και συνάφειας
    for (i = 0; i < num_processors; i++) {
        processors[i] = NULL;
        affinity_map[i] = NULL;
        processor_status[i] = PROC_NEW;
    }

    // Εγκατάσταση χειριστή για SIGCHLD
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_handler = 0;
    sig_act.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
    sig_act.sa_sigaction = sigchld_handler;
    sigaction(SIGCHLD, &sig_act, NULL);

    while (1) {
        // Εύρεση ελεύθερου επεξεργαστή ή επεξεργαστή με συνάφεια
        for (i = 0; i < num_processors; i++) {
            if (processors[i] == NULL || processors[i]->status == PROC_EXITED) {
                processors[i] = NULL;
            }
        }

        for (i = 0; i < num_processors; i++) {
            if (processors[i] == NULL) {
                proc_t *proc = proc_rq_dequeue();
                if (proc == NULL) continue; // Δεν υπάρχουν άλλες διεργασίες στην ουρά

                // Έλεγχος για συνάφεια
                if (affinity_map[i] != NULL && affinity_map[i] != proc) {
                    proc_to_rq_end(proc); // Δεν ταιριάζει, πίσω στην ουρά
                    continue;
                }

                proc->t_start = proc_gettime();
                pid = fork();

                if (pid == -1) {
                    err_exit("fork failed!");
                } else if (pid == 0) {
                    execl(proc->name, proc->name, NULL);
                } else {
                    proc->pid = pid;
                    proc->status = PROC_RUNNING;
                    processors[i] = proc;
                    affinity_map[i] = proc; // Ορισμός συνάφειας
                    processor_status[i] = PROC_RUNNING;

                    printf("Processor %d executing process %s (Affinity)\n", i + 1, proc->name);
                }
            }
        }

        // Εκ περιτροπής έλεγχος διεργασιών
        for (i = 0; i < num_processors; i++) {
            if (processors[i] && processors[i]->status == PROC_RUNNING) {
                nanosleep(&req, &rem); // Εκτέλεση για το κβάντο χρόνου

                if (processors[i]->status == PROC_RUNNING) {
                    kill(processors[i]->pid, SIGSTOP); // Διακοπή διεργασίας
                    processors[i]->status = PROC_STOPPED;
                    proc_to_rq_end(processors[i]); // Επιστροφή στην ουρά
                    processors[i] = NULL;
                }
            }
        }

        // Έλεγχος για ολοκλήρωση διεργασιών
        int active_processes = 0;
        for (i = 0; i < num_processors; i++) {
            if (processors[i] != NULL) active_processes++;
        }
        if (active_processes == 0 && proc_queue_empty(&global_q)) break;
    }
}
