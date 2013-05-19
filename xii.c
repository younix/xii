/*
 * Copyright (c) 2013 Jan Klemkow <j.klemkow@wemelug.de>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <X11/Intrinsic.h>	/* Include standard Toolkit Header file.
				 * We do not need "StringDefs.h" */
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>	/* Include the Label widget's header file. */
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/TextSink.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Cardinals.h>  /* Definition of ZERO. */

struct content {
	ssize_t size;
	char *data;
	Widget output;
	int fd;
};

void
file_input(XtPointer data, XtIntervalId *id)
{
	char buf[BUFSIZ];
	ssize_t size;
	struct content *content = data;

	while ((size = read(content->fd, buf, BUFSIZ)) > 0) {
		content->size += size;
		content->data = realloc(content->data, content->size);
		if (content->data == NULL)
			perror("realloc");
		strncat(content->data, buf, size);
		XtVaSetValues(content->output,XtNstring, content->data, NULL);
		XtVaSetValues(content->output, XtNinsertPosition,
		    strlen(content->data), NULL);
	}

	if (size == -1)
		perror("read");

	XtAppAddTimeOut(XtWidgetToApplicationContext(content->output), 100,
		file_input, data);
}

void
output(Widget input, XtPointer fd, XEvent *ev, Boolean *dispatch)
{
	Arg args[1];
	char *str = NULL;

	if (XLookupKeysym(&ev->xkey, 0) == XK_Return) {
		XtSetArg(args[0], XtNstring, &str);
		XtGetValues(input, args, 1);
		write(*(int*)fd, str, strlen(str));

		XtVaSetValues(input, XtNstring, "", NULL);
		*dispatch = FALSE;
	}
}

void
usage(void)
{
	fprintf(stderr, "xii [-t <title>] [-e <window>] <out> <in>\n");
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	XtAppContext app_con;
	struct content content;
	Window parent = 0;
	int ch ;
	char *title = "xii";

	while ((ch = getopt(argc, argv, "e:t:")) != -1) {
		switch (ch) {
		case 'e':
			errno = 0;
			parent = strtol(optarg, NULL, 0);
			if (errno != 0)
				perror("strtol");
			break;
		case 't':
			title = strdup(optarg);
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	char *outFileName = argv[0];
	char *inFileName = argv[1];

	/* set locale */
	XtSetLanguageProc(NULL, NULL, NULL);

	Widget toplevel = XtAppInitialize(&app_con, title, NULL, ZERO, &argc,
	    argv, NULL, NULL, ZERO);

	int i = 0;
	Arg args[10];

	Widget box = XtCreateManagedWidget("form", panedWidgetClass, toplevel,
	    args, i);

	/* create input field */
	i = 0;
	XtSetArg(args[i], XtNtype,		XawAsciiString);	i++;
	XtSetArg(args[i], XtNshowGrip,		FALSE);			i++;
	XtSetArg(args[i], XtNskipAdjust,	FALSE);			i++;
	XtSetArg(args[i], XtNwrap,		XawtextWrapLine);	i++;
	XtSetArg(args[i], XtNscrollHorizontal,	XawtextScrollNever);	i++;
	XtSetArg(args[i], XtNscrollVertical,	XawtextScrollNever);	i++;

	content.output = XtCreateManagedWidget("output", asciiTextWidgetClass,
	    box, args, i);

	/* create input field */
	i = 0;
	XtSetArg(args[i], XtNeditType,		XawtextEdit);		i++;
	XtSetArg(args[i], XtNskipAdjust,	TRUE);			i++;

	Widget input = XtCreateManagedWidget("input", asciiTextWidgetClass,
				box, args, i);

	/* handle input file */
	int out_fd;
	if ((out_fd = open(inFileName, O_WRONLY)) < 0)
		perror("open");

	XtAddEventHandler(input, KeyReleaseMask, TRUE, output, &out_fd);

	/* handel output file */
	if ((content.fd = open(outFileName, O_RDONLY)) < 0)
		perror("open");
	content.size = 1;
	content.data = calloc(1, content.size);

	XtAppAddTimeOut(app_con, 100, file_input, &content);

	XtRealizeWidget(toplevel);

	printf("window: %d\n", XtWindow(box));

	if (parent != 0)
		XReparentWindow(XtDisplay(box), XtWindow(box), parent, 0, 0);

	XtAppMainLoop(app_con);

	return EXIT_SUCCESS;
}
