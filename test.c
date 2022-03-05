#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <poll.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#define LENGTH(a)               (sizeof(a) / sizeof(a)[0])
#define die(str) do {fprintf(stderr, "error: line '%d': " str "\n", __LINE__);perror(" failed");exit(1);} while (0)

#define SIZE		1
#define CMDLENGTH	50

static int running = 1;
static int pipes[SIZE][2];
static char output[CMDLENGTH][SIZE];

int
main(int argc, char *argv[])
{
	static struct pollfd fds[SIZE] = {0};

	fds[0].fd = 0; /* 0 = STDIN_FILENO */
	fds[0].events = POLLIN;

	while (running) {
		//FIXME poll breaks (wakes up) whenever a signal is send to,
		//might wanna detect errno == EINTR
		if ((poll(fds, SIZE, -1)) == -1) {
			die("poll returned '-1'");
		}

		/* pipes polling */
		if (fds[0].revents & POLLIN) {
				char buffer[CMDLENGTH];
				int bt = read(pipes[0][0], buffer, LENGTH(buffer));
				strcpy(output[0], buffer);
				printf("string received! = '%s'\n", output[0]);
		} else if (fds[0].revents & POLLHUP) {
			die("pipe hangup");
		}
	}
	return 0;
}
