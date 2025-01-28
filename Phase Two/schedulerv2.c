//code goes here
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
#define MAX_PROCESSORS 100000 // Ο μέγιστος αριθμός επεξεργαστών

typedef struct proc_desc {
    struct proc_desc *next;
    struct proc_desc *prev;
    char name[80];
    int processors_needed;
    int pid;
    int status;
    double t_submission, t_start, t_end;
} proc_t;

struct single_queue {
    proc_t *first;
    proc_t *last;
    long members;
};

struct single_queue global_q;

int total_processors = 0;   // Συνολικοί διαθέσιμοι επεξεργαστές
int free_processors = 0;    // Ελεύθεροι επεξεργαστές

#define PROC_NEW       0
#define PROC_STOPPED   1
#define PROC_RUNNING   2
#define PROC_EXITED    3

#define proc_queue_empty(q) ((q)->first == NULL)

void proc_queue_init(struct single_queue *q) {
    q->first = q->last = NULL;
    q->members = 0;
}



void proc_to_rq_end(proc_t *proc) {
    proc->next = NULL; // Δεν υπάρχει επόμενος αφού μπαίνει στο τέλος

    if (proc_queue_empty(&global_q)) {
        global_q.first = global_q.last = proc;
        proc->prev = NULL; // Είναι ο πρώτος κόμβος, δεν έχει προηγούμενο
    } else {
        global_q.last->next = proc;   // Συνδέουμε τον προηγούμενο κόμβο με το νέο
        proc->prev = global_q.last;  // Ο προηγούμενος του νέου είναι ο παλιός τελευταίος
        global_q.last = proc;        // Ενημερώνουμε το τελευταίο στοιχείο της ουράς
    }
}



proc_t *proc_rq_dequeue() {
    proc_t *proc = global_q.first; // Παίρνουμε το πρώτο στοιχείο της ουράς
    if (proc == NULL) return NULL; // Αν η ουρά είναι κενή, επιστρέφουμε NULL

    global_q.first = proc->next; // Μετακινούμε τον δείκτη στον επόμενο κόμβο
    if (global_q.first == NULL) {
        global_q.last = NULL; // Αν αφαιρέθηκε το τελευταίο στοιχείο, η ουρά είναι πλέον κενή
    }

    proc->next = NULL; // Αποσυνδέουμε τον κόμβο από την ουρά
    return proc;       // Επιστρέφουμε τον κόμβο
}


double proc_gettime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (double)(tv.tv_sec + tv.tv_usec / 1000000.0);
}

void err_exit(char *msg) {
    printf("Error: %s\n", msg);
    fflush(stdout);
    exit(1);
}
// Λίστα για ενεργές διεργασίες
struct single_queue active_q;

void rr();
#define FCFS	0
#define RR	1

int policy = FCFS;
int quantum = 100;	
proc_t *running_proc;
double global_t;


void fcfs() {
    printf("Starting FCFS scheduling...\n");
    fflush(stdout);
    //int c;
    int status;
    proc_t *proc;
    proc_t *prev;
    proc_t *previous;
    pid_t pid;
    int active_processes = 0;

    while (!proc_queue_empty(&global_q)) {
        while (!proc_queue_empty(&global_q) && free_processors > 0) {
            proc = global_q.first;

            if (proc->processors_needed <= free_processors) {
                proc = proc_rq_dequeue(); 
                free_processors -= proc->processors_needed; 

                if (proc->status == PROC_NEW) {
                    proc->t_start = proc_gettime();
                    pid = fork();

                    if (pid == -1) {
                        err_exit("fork failed!");
                    }

                    if (pid == 0) {
                        execl(proc->name, proc->name, NULL);
                        err_exit("execl failed!");
                    } else {
                        proc->pid = pid;
                        proc->status = PROC_RUNNING;
                        printf("Started process: %s (PID %d), Processors: %d\n",
                               proc->name, pid, proc->processors_needed);
                        fflush(stdout);
                    }
                }
            } else {
                printf("\n");
            }
        }

        proc_t *completed_proc = NULL;
        if (pid > 0) {
            while ((pid = waitpid(-1, &status, 0)) > 0) {
            for (proc_t *p = global_q.last ; p != NULL; p =p->prev){
                if (p->pid == pid) {
                    completed_proc = p;
                    printf("Process with PID %d has finished.\n", pid);
                    fflush(stdout);

                    if (completed_proc) {
                
                        completed_proc->status = PROC_EXITED;
                        completed_proc->t_end = proc_gettime();
                        free_processors += completed_proc->processors_needed;

                        printf("PID %d - CMD: %s\n", pid, completed_proc->name);
                        fflush(stdout);
                        printf("\tElapsed time = %.2lf secs\n", completed_proc->t_end - completed_proc->t_submission);
                        fflush(stdout);
                        printf("\tExecution time = %.2lf secs\n", completed_proc->t_end - completed_proc->t_start);
                        fflush(stdout);
                    }
                    else  {
                        perror("waitpid failed");
                        break;
                    }
            
                }
            }
        }
    }
    
    printf("All processes completed.\n");
    fflush(stdout);
    }
    
}



int main(int argc,char **argv)
{
	FILE *input;
	char exec[80];
	int c, num_processors;
	//int numprocs;
    int processors_needed;
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
                total_processors = atoi(argv[3]);
            } else {
                total_processors = 1; // Default to 1 processor
            }
		} else if (!strcmp(argv[1],"RR")) {
			policy = RR;
			quantum = atoi(argv[2]);
			input = fopen(argv[3],"r");
			if (input == NULL) err_exit("invalid input file name");
			else {
			err_exit("invalid usage");
		    }
        }
	}
    proc_queue_init(&global_q);

    free_processors = total_processors;
    
	while ((c=fscanf(input, "%s %d", exec, &processors_needed))!=EOF) { ///////////////////////////////
		proc = malloc(sizeof(proc_t));
		proc->next = NULL;
		strcpy(proc->name, exec);
        proc->processors_needed = processors_needed > 0 ? processors_needed : 1; //changed
		proc->pid = -1;
		proc->status = PROC_NEW;
		proc->t_submission = proc_gettime();
		proc_to_rq_end(proc);
	}


    fclose(input);

  	global_t = proc_gettime();
	switch (policy) {
		case FCFS:
			fcfs();
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
