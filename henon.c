// (C) 2017 by folkert van heusden
// released under AGPL v3.0

#include <stdint.h>
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
	char fin = 0;
	char show_fps = 0;

	GC gc;
	Visual *visual;
	double pre, pim;

	double R = 0.0;
	char *image32;
	uint32_t *iu32;
	int image32_bytes;
	XImage *img;

	int *pixels, pixels_bytes;
	int max_ = -1;

	char fontName[128];
	Font f;

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

	image32_bytes = width * height * 4;
	image32 = (char *)calloc(image32_bytes, 1);
	iu32 = (uint32_t *)image32;
	img = XCreateImage(display, visual, 24, ZPixmap, 0, image32, width, height, 32, 0);

	pixels_bytes = width * height * sizeof(int);
	pixels = (int *)calloc(pixels_bytes, 1);

	snprintf(fontName, sizeof fontName, "-*-*-*-*-*-*-%d-*-*-*-*-*-iso8859-1", 32);
	f = XLoadFont(display, fontName);

	double ta, tb, pa = 0, pb = 0;
	double duration = 5.0;
	for(;;)
	{
		double add_a, add_b, x = 1, y = 1;
		double start_ts = get_ts(), end_ts = start_ts + duration, now_ts = start_ts;
		int frames = 0;
		double use_it = max_it;

		ta = 1.3 + (rand() % 1000) / 10000.0;
		tb = 0.0 + (rand() % 3000) / 6000.0;

		printf("%f, %f\n", ta, tb);

		add_a = ta - pa;
		add_b = tb - pb;

		while((now_ts = get_ts()) < end_ts && !fin)
		{
			int i;
			double time_passed = now_ts - start_ts;
			double fraction_passed = time_passed / (double)duration;

			double a = pa + add_a * fraction_passed;
			double b = pb + add_b * fraction_passed;

			///
			memset(pixels, 0x00, pixels_bytes);

			max_ = -1;
			for(i=0; i<4000; i++) {
				int X, Y;
				double x2 = 1 - a * x * x + y;

				y = b * x;
				x = x2;

				X = (x + 2.0) / 4.0 * width;
				Y = (y + 2.0) / 4.0 * height;

				if (X < 0) X = 0;
				if (X >= width) X = width - 1;
				if (Y < 0) Y = 0;
				if (Y >= height) Y = height - 1;

				int o = Y * width + X;
				pixels[o]++;

				if (pixels[o] > max_)
					max_ = pixels[o];
			}

			memset(image32, 0x00, image32_bytes);
			for(int i=0; i<width * height; i++)
				iu32[i] = pixels[i] * (1677215/max_);

			XPutImage(display, window, gc, img, 0, 0, 0, 0, width, height);
			//

			frames++;
			double r_time_passed = now_ts - start_ts;
			double fps = r_time_passed ? frames / r_time_passed : target_fps;

			if (show_fps) {
				char temp[16];
				snprintf(temp, sizeof temp, "%.1f", fps);
				// holder for a text
				XTextItem xti;
				xti.chars = temp;
				xti.nchars = strlen(xti.chars);
				xti.delta = 0;
				xti.font = f;

				XSetForeground(display, gc, WhitePixelOfScreen(DefaultScreenOfDisplay(display)));
				XSetFillStyle(display, gc, FillSolid);
				XDrawText(display, window, gc, 32, 32, &xti, 1);
			}

			use_it = fps / target_fps * max_it;

			if (use_it < 1)
				use_it = 1;
			else if (use_it > 100)
				use_it = 100;

			while(XPending(display)) {
				XEvent e;
				XNextEvent(display, &e);
				if (e.type == ClientMessage) {
					fin = 1;
					break;
				}
			}
		}

		pa = ta;
		pb = tb;
	}

// a: 1.05 - 1.145
// b: -1.5 - 1.5

	XDestroyImage(img);

	return 0;
}
