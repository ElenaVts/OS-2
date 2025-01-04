# OS-2
OS Project

Process Scheduler

1. Introduction

1.1 Objective of the Assignment The objective of the assignment is to extend a scheduling environment in a Unix operating system for a given number of processors. Specifically, a scheduler needs to be extended to take as input the applications to be executed, reading a file with their names and computational requirements (number of processors). These applications are to be stored in an appropriate data structure (list) and then scheduled using one of the following policies:

  First-Come, First-Served (FCFS): Applications are executed in the order they are inserted into the list, i.e., in the order they are listed in the input file.
  Round Robin (RR): Applications are executed in turns for a time equal to a predefined scheduling quantum.
  Strictly Affined Round Robin (RR-AFF): Like the previous policy, but an application can only execute on the processor it was initially assigned when it first ran.

1.2 Work Data You are provided with an implementation of a scheduler for the two basic scheduling policies (FCFS, RR) under the assumption of a single processor in the system.
The scheduler's implementation has been designed so that its core data structures and internal operations are as compact and independent of the policy being applied as possible. For instance, the policy could change dynamically at runtime. This section provides a general description of the scheduler's implementation basics.

1.2.1 Application Descriptor Each application to be executed by the scheduler is described by a suitable data structure. This structure is allocated and initialized appropriately for each application read from the input file, to be executed by the scheduler. It is then inserted into the execution queue. This structure includes fields describing the application's characteristics, such as the executable file name, process ID (pid), execution state, queue entry time, etc.

Possible execution states for an application (process) are:

  NEW: The application has just been added to the execution queue. Applications are executed via the scheduler, which creates the process for an application's execution (using fork-exec).
  RUNNING: The application (process) is active.
  STOPPED: The application's execution has been suspended.
  EXITED: The application has terminated.

1.2.2 Execution Queue The data structure containing the application descriptors is a linked list (queue). Processes are added to the end of the list, and removed from the front.

1.2.3 Application Scheduling The scheduler selects the next application for execution each time, and if there is an application, it performs the necessary actions; otherwise, it terminates. Applications to be scheduled are assumed to involve only computations with no input/output instructions.

Thus, in the case of static scheduling policy (FCFS), the scheduler activates only when an application (process) terminates.
Conversely, in the case of dynamic scheduling policy (RR), the scheduler activates at regular intervals, as defined by the scheduling quantum, which is a parameter of the program. The scheduler's activation mechanism is implemented using an appropriate timer (e.g., nanosleep call).

For dynamic scheduling policies, the management of application (process) execution, i.e., suspending and resuming their execution, is handled using the SIGSTOP and SIGCONT signals.
Finally, when an application (process) terminates, the scheduler is notified using a handler for the SIGCHLD signal to execute the necessary actions for its proper operation.

Whenever an application terminates, the total execution time is calculated and printed, starting from the moment it was added to the execution list until its completion.
For simplicity, all processes are inserted into the system, i.e., the execution queue, before the scheduling policy execution starts (i.e., at the corresponding "0" time, according to theory exercises).

1.2.4 Additional Information

  The scheduler receives the SIGCHLD signal only when a child application terminates, not when its execution is suspended (see sigaction).
  The scheduler maintains a pointer to the process descriptor of the application running at any given time.
  The scheduler displays execution information for each completed application and the total execution time for all applications.
  Test program code, sample input files, a script with examples of scheduler execution, a sample output example, and the initial scheduler code (which you need to extend) are provided along with the assignment description.

1.2.5 Usage According to the above, the scheduler program is used as follows: scheduler <policy> [<quantum>] <input_filename>, where:

  scheduler: the executable of the scheduler you will implement.
  policy: the scheduling policy to execute the applications. Valid values are FCFS and RR.
  quantum: The scheduling quantum in milliseconds, required only for the RR scheduling policy.
  input_filename: The file name containing the workload to be executed via the scheduler. Each line of the file includes the name of an application's executable.

Examples:

$ scheduler FCFS homogeneous.txt
$ scheduler FCFS reverse.txt
$ scheduler RR 1000 homogeneous.txt
$ scheduler RR 1000 reverse.txt

2. Phase One: Supporting Multiple Processors

Until now, the scheduler assumes there is a single processor in the system, and each process requests a single processor. In the first phase of implementation, you need to extend the scheduler to manage multiple processors, while applications continue to request only one processor. The number of processors in the system will be provided as an argument to the scheduler program. The requirements for each application's processors will be specified in the workload file, alongside the application's executable name. If not specified, an application requests one processor.
The scheduler will take applications as input by reading a file with their names and computational requirements (number of processors). These are inserted into an appropriate data structure (list) and then scheduled using one of the following policies:

  First-Come, First-Served (FCFS): Applications are executed in the order they are listed in the input file. If the requirements of an application cannot be met (e.g., it requests both processors while only one is available), the scheduler may look for the next available and suitable application, if any.
  Round Robin (RR): Applications are executed in turns for a predefined scheduling quantum. Similar to before, the scheduler may look for the next process whose requirements can be satisfied by the available processors.
  Strictly Affined Round Robin (RR-AFF): Similar to the previous policy, but an application can only execute on the processors initially assigned during its first execution.

Hints:

  The maximum number of active processes equals the number of processors.
  The FCFS policy is equivalent to RR with an infinite (practically very large) quantum.
  The SIGCHLD signal handler allows asynchronous handling of application termination.

3. Phase Two: Supporting Multi-Processor Requests

Extend your previous implementation so that each process can request a number of processors less than or equal to the available processors managed by the scheduler. The scheduling mechanisms remain the same, but the assignment of processors to applications changes. Implement and evaluate this extension for the FCFS policy only.
