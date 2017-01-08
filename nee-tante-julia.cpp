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

inline int calc_julia_pixel(double x, double y, const double cre, const double cim, const int mit)
{
	int it = 0;

	do
	{
		double px = x * x;
		double py = y * y;

		if (px + py >= 4.0)
			break;

		double z = px - py + cre;
		y = 2.0 * x * y + cim;
		x = z;
	}
	while(++it < mit);

	return it;
}

void julia(char *const rgb_out, const int w, const int h, const double re, const double im, const int mit)
{
	int *const pixels = (int *)malloc(w * h * 3 * sizeof(int)), *p = pixels;

	int mr = 0, mg = 0, mb = 0;
	for(int y=0; y<h; y++)
	{
		const double yc = (double)y * 4.0 / (double)h - 2.0;

		for(int x=0; x<w; x++)
		{
			const double xc = (double)x * 4.0 / (double)w - 2.0;

			int cr = calc_julia_pixel(xc, yc, re, re, mit);
			*p++ = cr;
			if (cr > mr)
				mr = cr;

			int cg = calc_julia_pixel(xc, yc, re, im, mit);
			*p++ = cg;
			if (cg > mg)
				mg = cg;

			int cb = calc_julia_pixel(xc, yc, im, im, mit);
			*p++ = cb;
			if (cb > mb)
				mb = cb;
		}
	}

	double mrs = 255.0 / double(mr);
	double mgs = 255.0 / double(mg);
	double mbs = 255.0 / double(mb);

	p = pixels;
	for(int y=0; y<h; y++)
	{
		char *out = &rgb_out[y * w * 4];

		for(int x=0; x<w; x++)
		{
			*out++ = (double)*p++ * mrs;
			*out++ = (double)*p++ * mgs;
			*out++ = (double)*p++ * mbs;
			*out++ = 255;
		}
	}

	free(pixels);
}

XImage *create_ximage(Display *display, Visual *visual, int width, int height, double re, double im, const int mit)
{
	char *image32 = (char *)malloc(width * height * 4);

	julia(image32, width, height, re, im, mit);

	return XCreateImage(display, visual, 24, ZPixmap, 0, image32, width, height, 32, 0);
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
	printf("nee-tante-julia is (C) 2017 by Folkert van Heusden\n");
	printf("Released under AGPL v3.0\n");

	Display *display = XOpenDisplay(getenv("DISPLAY"));

	const char *e = getenv("XSCREENSAVER_WINDOW");

	int width = 640, height = 480;
	Window window;

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

	GC gc = XCreateGC(display, window, 0, NULL);

	Visual *visual = DefaultVisual(display, 0);

	XSetErrorHandler(x11_err_handler);

	double pre = 0.0, pim = 0.0;

	srand48(time(NULL));

	XSetErrorHandler(x11_err_handler);

	for(;;)
	{
		double re = drand48() * 4.0 - 2.0;
		double im = drand48() * 4.0 - 2.0;

		//printf("%f,%f %f,%f\n", pre, pim, re, im);
		double add_re = re - pre;
		double add_im = im - pim;

		double start_ts = get_ts(), end_ts = start_ts + duration, now_ts = start_ts;
		int frames = 0;

		double use_it = max_it;
		while((now_ts = get_ts()) < end_ts)
		{
			double time_passed = now_ts - start_ts;
			double fraction_passed = time_passed / (double)duration;

			double cur_re = pre + add_re * fraction_passed;
			double cur_im = pim + add_im * fraction_passed;

			XImage *ximage = create_ximage(display, visual, width, height, cur_re, cur_im, int(use_it));
			XPutImage(display, window, gc, ximage, 0, 0, 0, 0, width, height);
			XDestroyImage(ximage);

			frames++;
			double r_time_passed = now_ts - start_ts;
			double fps = r_time_passed ? frames / r_time_passed : target_fps;

			use_it = fps / target_fps * max_it;

			if (fps > target_fps)
				usleep(1000);
		}

		pre = re;
		pim = im;
	}

	return 0;
}
