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
        pid_t pid = atoi(argv[1]);
        long ret;


        ret = syscall(SYS_yfkm2_monitor, pid);

	if (ret == 0){
		printf("Sucess on adding %d!\n", pid);
		return 0;
	} else {
		printf("Failure! Is %s a valid PID?\n", argv[1]);
		return 1;
	}
}
