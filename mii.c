#include <Xm/Xm.h> 
#include <Xm/PushB.h>

/* Prototype Callback function */
void pushed_fn(Widget, XtPointer, XmPushButtonCallbackStruct *);

void
pushed_fn(Widget w, XtPointer client_data, XmPushButtonCallbackStruct *cbs) 
{   
	printf("Don't Push Me!!\n");
}

int
main(int argc, char **argv) 
{
	Widget top_wid, button;
	XtAppContext  app;

	top_wid = XtVaAppInitialize(&app, "Push", NULL, 0,
			&argc, argv, NULL, NULL);

	button = XmCreatePushButton(top_wid, "Push_me", NULL, 0);

	/* tell Xt to manage button */
	XtManageChild(button);

	/* attach fn to widget */
	XtAddCallback(button, XmNactivateCallback, pushed_fn, NULL);

	XtRealizeWidget(top_wid); /* display widget hierarchy */
	XtAppMainLoop(app); /* enter processing loop */ 

}
