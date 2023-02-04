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

const int max_it = 64;
const double duration = 5;
const int target_fps = 25;

double get_ts(void)
{
        struct timeval ts;

        gettimeofday(&ts, NULL);

        return (double)ts.tv_sec + (double)ts.tv_usec / 1000000.0;
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

	const char *e = getenv("XSCREENSAVER_WINDOW");

	int width = 640, height = 480;
	Window window;

	GC gc;
	Visual *visual;
	double pre, pim;

	double R = 0.0;
	char *image32;
	XImage *img;

	printf("bifurcation is (C) 2017 by Folkert van Heusden\n");
	printf("Released under AGPL v3.0\n");

	if (e) {
		window = strtol(e, NULL, 16);

		XWindowAttributes wa;
		XGetWindowAttributes(display, window, &wa);

		width = wa.width;
		height = wa.height;
	}
	else {
		RootWindow(display, DefaultScreen(display));

		int screen_num = DefaultScreen(display);

		window = XCreateSimpleWindow(display,
                          RootWindow(display, screen_num),
                          0, 0,
                          width, height,
                          0, BlackPixel(display, screen_num),
                          WhitePixel(display, screen_num));

		XMapWindow(display, window);
	}

	gc = XCreateGC(display, window, 0, NULL);

	visual = DefaultVisual(display, 0);

	XSetErrorHandler(x11_err_handler);

	pre = pim = 0.0;

	srand48(time(NULL));

	XSetErrorHandler(x11_err_handler);

	image32 = (char *)calloc(width * height * 4, 1);
	img = XCreateImage(display, visual, 24, ZPixmap, 0, image32, width, height, 32, 0);

	for(;;)
	{
		double v = 0.5;
		int b, y;

		XPutImage(display, window, gc, img, 0, 0, 0, 0, width, height);

		for(y=0; y<height; y++) {
			int o = y * width * 4;
			memmove(&image32[o], &image32[o + 4], (width - 1) * 4);
		}

		for(b=0; b<1000; b++) {
			int y = v * height;

			if (y >= 0 && y < height) {
				int o = y * width * 4 + (width - 1) * 4;
				image32[o]++;
			}

//printf("%f %f\n", R, v);
			v = R * v * (1.0 - v);
		}

		R += 0.001;

		usleep(10000);
	}

	XDestroyImage(img);

	return 0;
}
