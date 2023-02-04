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

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

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

	snprintf(fontName, sizeof fontName, "-*-*-*-*-*-*-%d-*-*-*-*-*-iso8859-1", 32);
	f = XLoadFont(display, fontName);

double min_x = 1000000, min_y = min_x, min_z = min_x;
double max_x = -1000000, max_y = max_x, max_z = max_x;

	double psz = 0, pa = 0, pab = 0, pXO = 0, pYO = 0, pXw = 0, pYw = 0;
	double duration = 5.0;
	for(;;)
	{
		double add_sz, add_a, add_ab, add_XO, add_YO, add_Xw, add_Yw;
		double start_ts = get_ts(), end_ts = start_ts + duration, now_ts = start_ts;
		int frames = 0;
		double use_it = max_it;

		double sz = (rand() % 262144) / 262143.0 * 2.0 - 1.0;
		double a = (rand() % 262144) / 262143.0 * 10.0;
		double ab = (rand() % 262144) / 262143.0 * 4.26;
		double XO = (rand() % 262144) / 262143.0 * 2.0 - 1.0;
		double YO = (rand() % 262144) / 262143.0 * 2.0 - 1.0;
		double Xw = (rand() % 262144) / 262143.0 * 2.0;
		double Yw = (rand() % 262144) / 262143.0 * 2.0;

		add_sz = sz - psz;
		add_a = a - pa;
		add_ab = ab - pab;
		add_XO = XO - pXO;
		add_YO = YO - pYO;
		add_Xw = Xw - pXw;
		add_Yw = Yw - pYw;

		while((now_ts = get_ts()) < end_ts && !fin)
		{
			int i;
			double time_passed = now_ts - start_ts;
			double fraction_passed = time_passed / (double)duration;

			double Z = psz + add_sz * fraction_passed;
			double A = pa + add_a * fraction_passed;
			double AB = pab + add_ab * fraction_passed;
			double cXO = pXO + add_XO * fraction_passed;
			double cYO = pYO + add_YO * fraction_passed;
			double cXw = pXw + add_Xw * fraction_passed;
			double cYw = pYw + add_Yw * fraction_passed;

			double dx = cXw / width - cXw / 2.0;
			double dy = cYw / height - cYw / 2.0;
			double h = 0.01, a = 5.0 + A, b = 24.74 + AB, c = 8.0 / 3.0;

		//printf("%f %f %f %f,%f\n", Z, A, b, cXO, cYO);

			///
			for(int Y=0; Y<height; Y++) {
				for(int X=0; X<width; X++) {
					double y = dy * Y + cYO;
					double x = dx * X + cXO;
					double z = Z;

					for(int i=0; i<10; i++) {
						double xt = x + h * a * (y - x);
						double yt = y + h * (x * (b - z) - y);
						double zt = z + h * (x * y - c * z);
						x = xt;
						y = yt;
						z = zt;
					}

					//printf("%f %f %f\n", x, y, z);

#if 0
					min_x = min(x, min_x);
					min_y = min(y, min_y);
					min_z = min(z, min_z);

					max_x = max(x, max_x);
					max_y = max(y, max_y);
					max_z = max(z, max_z);
#endif
					uint8_t r = (x + 30.0) / 60.0 * 255.0;
					uint8_t g = (y + 30.0) / 60.0 * 255.0;
					uint8_t b = z / 60.0 * 255.0;
					iu32[Y * width + X] = ((r << 16) | (g << 8) | b);
				}
			}

//printf("%f %f %f\n", min_x, min_y, min_z);
//printf("%f %f %f\n", max_x, max_y, max_z);

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

		psz = sz;
		pa = a;
		pab = ab;
		pXO = XO;
		pYO = YO;
		pXw = Xw;
		pYw = Yw;
	}

// a: 1.05 - 1.145
// b: -1.5 - 1.5

	XDestroyImage(img);

	return 0;
}
