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
Fokus(Widget w, XEvent *ev, String *str, Cardinal *car)
{
	printf("FOKUS_IN\n");
}

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


//		XtCallActionProc(content->output, "end-of-file",
//		    NULL, NULL, 0);

//		XawTextScroll(content->output, 10, 0);

//		for (int i = 0; i < 100; i++)
//			XtCallActionProc(content->output,
//			    "scroll-one-line-down", NULL, NULL, 0);

//		XtVaSetValues(content->output, XtNinsertPosition,
//		    strlen(content->data), NULL);

		XtVaSetValues(content->output, XtNdisplayPosition, 100, NULL);
	}

	if (size == -1)
		perror("read");

	XtAppAddTimeOut(XtWidgetToApplicationContext(content->output), 100,
		file_input, data);
}

void
output(Widget input, XtPointer in_file, XEvent *ev, Boolean *dispatch)
{
	Arg args[1];
	char *str = NULL;
	int fd = STDIN_FILENO;

	if (XLookupKeysym(&ev->xkey, 0) == XK_Return) {
		XtSetArg(args[0], XtNstring, &str);
		XtGetValues(input, args, 1);

		if (strcmp((char*)in_file, "-") != 0)
			if ((fd = open((char*)in_file, O_WRONLY)) < 0)
				perror("open");

		write(fd, str, strlen(str));

		close(fd);

		XtVaSetValues(input, XtNstring, "", NULL);
		*dispatch = FALSE;
	}
}

static int
error_handler(Display *display, XErrorEvent *error)
{
	printf("error: %d\n", error->error_code);

	return 0;
}

void
usage(void)
{
	fprintf(stderr,
	    "xii [-t <title>] [-e <window>] [-d <dir>] [-o <out>] [-i <in>]\n");
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
	char *out_file = "out";
	char *in_file = "in";

	while ((ch = getopt(argc, argv, "hi:o:e:t:")) != -1) {
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
		case 'o':
			out_file = strdup(optarg);
			break;
		case 'i':
			in_file = strdup(optarg);
			break;
		case 'h':
		default:
			usage();
			/* NOTREACHED */
		}
	}

	/* set locale */
	XtSetLanguageProc(NULL, NULL, NULL);

	Widget toplevel = XtAppInitialize(&app_con, title, NULL, ZERO, &argc,
	    argv, NULL, NULL, ZERO);

//	XtActionsRec actions[] = {
//		{"Fokus", Fokus},
//	};

//	XtAddActions(actions, XtNumber(actions));

//	XtOverrideTranslations(toplevel,
//	    XtParseTranslationTable("<Message>XEMBED_FOCUS_IN: Fokus()"));

	int i = 0;
	Arg args[20];

	Widget box = XtCreateManagedWidget("form", panedWidgetClass, toplevel,
	    args, i);

	/* create output field */
	i = 0;
	XtSetArg(args[i], XtNtype,		XawAsciiString);	i++;
	XtSetArg(args[i], XtNeditType,		XawtextRead);		i++;
	XtSetArg(args[i], XtNshowGrip,		FALSE);			i++;
//	XtSetArg(args[i], XtNskipAdjust,	FALSE);			i++;
	XtSetArg(args[i], XtNwrap,		XawtextWrapLine);	i++;
//	XtSetArg(args[i], XtNscrollHorizontal,	XawtextScrollNever);	i++;
	XtSetArg(args[i], XtNscrollHorizontal,	XawtextScrollWhenNeeded); i++;
	XtSetArg(args[i], XtNscrollVertical,	XawtextScrollWhenNeeded); i++;

	content.output = XtCreateManagedWidget("output", asciiTextWidgetClass,
	    box, args, i);

	/* create input field */
	i = 0;
	XtSetArg(args[i], XtNeditType,		XawtextEdit);		i++;
	XtSetArg(args[i], XtNskipAdjust,	TRUE);			i++;

	Widget input = XtCreateManagedWidget("input", asciiTextWidgetClass,
	    box, args, i);

	/* don't do new-line on return */
//	XtOverrideTranslations(input,
//	    XtParseTranslationTable("<Key>Return: Nothing()"));

	/* handle input file */

	XtAddEventHandler(input, KeyReleaseMask, TRUE, output, in_file);
	XSetErrorHandler(error_handler);

	/* handel output file */
	if ((content.fd = open(out_file, O_RDONLY)) < 0)
		perror("open");
	content.size = 1;
	content.data = calloc(1, content.size);

	XtAppAddTimeOut(app_con, 100, file_input, &content);

	XtRealizeWidget(toplevel);
	XtSetKeyboardFocus(toplevel, input);

	if (parent != 0)
		XReparentWindow(XtDisplay(toplevel), XtWindow(toplevel), parent,
		    0, 0);

	XtAppMainLoop(app_con);

	return EXIT_SUCCESS;
}
