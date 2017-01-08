// (C) 2017 by folkert van heusden
// released under AGPL v3.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

void draw(char *rgb_out, int w, int h)
{
	for(int i=0; i<w*h*4; i++)
		*rgb_out++ = rand();
}

XImage *create_ximage(Display *display, Visual *visual, int width, int height)
{
	char *image32 = (char *)malloc(width * height * 4);

	draw(image32, width, height);

	return XCreateImage(display, visual, DisplayPlanes(display, DefaultScreen(display)), ZPixmap, 0, image32, width, height, 32, 0);
}

int x11_err_handler(Display *pd, XErrorEvent *pxev)
{
	char msg[4096] = { 0 };

	XGetErrorText(pd, pxev -> error_code, msg, sizeof(msg));

	printf("%s\n", msg);

	return 0;
}

int main(int argc, char **argv)
{
	Display *display = XOpenDisplay(getenv("DISPLAY"));

	Window window = strtol(getenv("XSCREENSAVER_WINDOW"), NULL, 16);

	XWindowAttributes wa;
	XGetWindowAttributes(display, window, &wa);
	int width = wa.width, height = wa.height;
	printf("res: %dx%d\n", width, height);

	GC gc = XCreateGC(display, window, 0, NULL);

	Visual *visual = DefaultVisual(display, 0);

	XSetErrorHandler(x11_err_handler);

#if 0
	for(;;)
	{
		XImage *ximage = create_ximage(display, visual, width, height);
		XPutImage(display, window, gc, ximage, 0, 0, 0, 0, width, height);
		XDestroyImage(ximage);

		usleep(1000);
	}
#endif

	XSetForeground(display, gc, WhitePixelOfScreen(DefaultScreenOfDisplay(display)));

	int th = height * 3 / 4;

	// load font with a size
	char fontName[128];
	snprintf(fontName, sizeof fontName, "-*-*-*-*-*-*-%d-*-*-*-*-*-iso8859-1", th);

	Font f = XLoadFont(display, fontName);

	// holder for a text
	XTextItem xti;
	xti.chars = "Maar hallo, dit is een test.";
	xti.nchars = strlen(xti.chars);
	xti.delta = 0;
	xti.font = f;

	// determine width in pixels of a string
	XFontStruct *fs = XQueryFont(display, f);
	int s = XTextWidth(fs, xti.chars, xti.nchars);

	// create an image structure
	int depth = DefaultDepth(display, DefaultScreen(display));
	Pixmap pixmap = XCreatePixmap(display, window, s, th + fs -> descent, depth);

	printf("descent: %d\n", fs -> descent);
	XDrawText(display, pixmap, gc, 0, th - fs -> descent, &xti, 1);

	int x = width - 1;
	for(;;) {
		XCopyArea(display, pixmap, window, gc, 0, 0, s, th, x, 0);

		x -= 10;

		if (x <= -s)
			 x = width - 1;

		XFlush(display);

		usleep(10000);
	}

	return 0;
}
