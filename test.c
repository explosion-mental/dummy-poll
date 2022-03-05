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

#define SIZE		2
#define CMDLENGTH	50

static int running = 1;
static int pipes[SIZE][2];
static char output[CMDLENGTH][SIZE];

static void
getcmd(int i, char *cmd)
{
	if (fork() == 0) {
		close(pipes[i][0]);
		dup2(pipes[i][1], STDOUT_FILENO);
		close(pipes[i][1]);
		execl("/bin/sh", "sh", "-c", cmd, (char*) NULL);
		exit(EXIT_SUCCESS);
	}
}

int
main(int argc, char *argv[])
{
	static struct pollfd fds[SIZE] = {0};

	fds[0].fd = STDIN_FILENO; /* 0 = STDIN_FILENO */
	fds[0].events = POLLIN;

	pipe(pipes[0]);
	fds[1].fd = pipes[0][0]; /* reading end */
	fds[1].events = POLLIN;

	while (running) {
		if ((poll(fds, SIZE, -1)) == -1)
			die("poll returned '-1'");

		/* main, stdin */
		if (fds[0].revents & POLLIN) {
				char buffer[CMDLENGTH] = {0};
				int bt = read(STDIN_FILENO, buffer, LENGTH(buffer));
				if (buffer[bt - 1] == '\n') /* chop off ending new line, if one is present */
					buffer[bt - 1] = '\0';
				if (!strcmp(buffer, "getcmd"))
					getcmd(0, "echo 'HELLO WORLD'");
				strcpy(output[0], buffer);
				printf("string received! = '%s'\n", output[0]);
		} else if (fds[0].revents & POLLHUP)
			die("main pipe hangup");

		/* pipes events */
		if (fds[1].revents & POLLIN) {
				char buffer[CMDLENGTH] = {0};
				int bt = read(fds[1].fd, buffer, LENGTH(buffer));
				if (buffer[bt - 1] == '\n') /* chop off ending new line, if one is present */
					buffer[bt - 1] = '\0';
				strcpy(output[1], buffer);
				printf("GETCMD READ: '%s'\n", output[1]);
		} else if (fds[1].revents & POLLHUP)
			die("pipe hangup");
	}

	return 0;
}
