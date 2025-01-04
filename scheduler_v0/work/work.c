#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

double a = 1.1;

void core_delay()
{
	unsigned long j;

	for (j = 0; j < 100000; j++) {
		a += sqrt(1.1)*sqrt(1.2)*sqrt(1.3)*sqrt(1.4)*sqrt(1.5);
		a += sqrt(1.6)*sqrt(1.7)*sqrt(1.8)*sqrt(1.9)*sqrt(2.0);
		a += sqrt(1.1)*sqrt(1.2)*sqrt(1.3)*sqrt(1.4)*sqrt(1.5);
		a += sqrt(1.6)*sqrt(1.7)*sqrt(1.8)*sqrt(1.9);
	}
}

void delay(int workload)
{
	int i;
	int total_workload = workload*DELAY;

	for (i = 0; i < total_workload; i++)
		core_delay();
}

int main(int argc, char **argv)
{
	int workload = LOAD;
	int pid = getpid();

	printf("process %d begins\n", pid);
	delay(workload);
	printf("process %d ends\n", pid);

	return 0;
}
