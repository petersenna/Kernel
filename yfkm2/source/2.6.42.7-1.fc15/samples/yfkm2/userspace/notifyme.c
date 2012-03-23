#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_yfkm2_monitor	__NR_yfkm2_monitor
#define SYS_yfkm2_notifyme	__NR_yfkm2_notifyme

int main (int argc, char *argv[])
{

	if (argc < 2) {
		printf("Error. Use %s <PID>\n", argv[0]);
		return 1;
	}

        pid_t monitor, notifyme;
        long ret;

	monitor = atoi(argv[1]);
	notifyme = getpid();

        ret = syscall(SYS_yfkm2_notifyme, monitor, notifyme);

	if (ret == 0){
		printf("Sucess on adding %d!\n", monitor);
		printf("Finish %d to see what happens to me.\n", monitor);
		while (1)
			sleep (10);
	} else {
		printf("Failure! Is %s a valid PID?\n", argv[1]);
		return 1;
	}
}
