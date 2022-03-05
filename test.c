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

#define SIZE		2
#define CMDLENGTH	50

#define die(str)     do { fprintf(stderr, "error: line '%d': " str "\n", __LINE__);perror(" failed");exit(1);} while (0)

static int running = 0;
static int pipes[SIZE - 1][2];
static char output[CMDLENGTH][SIZE - 1];

int
main(int argc, char *argv[])
{
	Display *dpy;
	Window win;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "xwindow: Unable to open display");
		exit(1);
	}

	static struct pollfd fds[SIZE] = {0};

	fds[0].fd = ConnectionNumber(dpy);
	fds[0].events = POLLIN;

	pipe(pipes[0]);
	fds[1].fd = pipes[0][0]; /* track read end only */
	fds[1].events = POLLIN;

	/* init window */
        win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 100, 100, 400, 400, 0, 0, 0);
	XSelectInput(dpy, win, KeyPressMask); // only listen to keypress
	XStoreName(dpy, win, "test");
	XMapRaised(dpy, win);

	/* main event loop */
	XFlush(dpy);
	while (running) {
		//FIXME poll breaks (wakes up) whenever a signal is send to,
		//might wanna detect errno == EINTR
		if ((poll(fds, SIZE, -1)) == -1) {
			die("poll returned '-1'");
		}

		/* X fd */
		//FIXME might wanna use `XCheckWindowEvent`, since it returns (doesn't block) if there is no events, you have to tell which events tho.
		if (fds[0].revents & POLLIN) {
			XEvent ev;
			while (running && XPending(dpy)) {
				XNextEvent(dpy, &ev);
				KeySym key;
				switch(ev.type) {
				case KeyPress: /* only handle keypress */
					key = XLookupKeysym(&ev.xkey, 0);
					switch(key) {
					case XK_q: /* quit */
						running = 0;
						break;
					case XK_j: //TODO
						//getcmd(i);
						break;
					}
					break;
				}
			}
		} else if (fds[0].revents & POLLHUP) {
			die("main loop event hangup");
		}

		/* pipes polling */
		if (fds[1].revents & POLLIN) {
				char buffer[CMDLENGTH];
				int bt = read(pipes[0][0], buffer, LENGTH(buffer));

				// Trim UTF-8 characters properly
				int j = bt - 1;
				//while ((buffer[j] & 0b11000000) == 0x80)
					//j--;
				// Cache last character and replace it with a trailing space
				char ch = buffer[j];
				buffer[j] = ' ';
				// Trim trailing spaces
				while (buffer[j] == ' ')
					j--;
				buffer[j + 1] = '\0';

				if (bt == LENGTH(buffer)) // Clear the pipe
					while (ch != '\n' && read(pipes[0][0], &ch, 1) == 1);

				strcpy(output[0], buffer);
		} else if (fds[1].revents & POLLHUP) {
			die("pipe hangup");
		}
	}
	XCloseDisplay(dpy);
	return 0;
}
