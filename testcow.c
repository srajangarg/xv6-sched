#include "types.h"
#include "user.h"

int main()
{	
	int x = 1;
	printf(1, "frames before fork 1 : %d\n", getNumFreePages());
	int pid1 = fork();
	if (pid1 == 0) {
		// x = 2;
		printf(1, "%d frames after fork 1 : %d\n", x, getNumFreePages());
		// int pid2 = fork();
		// if (pid2 == 0) {
		// 	printf(1, "frames after fork 2 : %d\n", getNumFreePages());

		// } else {
		// 	wait();
		// 	printf(1, "frames after wait 2 : %d\n", getNumFreePages());
		// }
	} else {
		wait();
		printf(1, "%d frames after wait 1 : %d\n", x, getNumFreePages());
	}
	exit();
}