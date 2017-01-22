// (C) 2017 by folkert van heusden
// released under AGPL v3.0

#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

const int max_it = 64;
const double duration = 5;
const int target_fps = 25;
std::string mqtt_topic = "/tickers/idle", mqtt_host = "192.168.64.1";
int mqtt_port = 1883;

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

int getFontHeight(Display *const display, const int reqHeight)
{
	char fontName[128];
	snprintf(fontName, sizeof fontName, "-*-*-*-*-*-*-%d-*-*-*-*-*-iso8859-1", reqHeight);

	Font f = XLoadFont(display, fontName);
	XFontStruct *fs = XQueryFont(display, f);

	const char *const test = "_()[]$#&Yy,/|~`^\"'";

	XCharStruct cs;
	int dir = 0, asc = 0, desc = 0;
	XTextExtents(fs, test, strlen(test), &dir, &asc, &desc, &cs);

	XUnloadFont(display, f);

	int h = asc + desc;

	return (reqHeight * reqHeight) / h;
}

void show_text(Display *const display, Window window, const int width, const int height, GC gc, const std::string & what)
{
	int th = height * 3 / 4;

	// load font with a size
	char fontName[128];
	snprintf(fontName, sizeof fontName, "-*-*-*-*-*-*-%d-*-*-*-*-*-iso8859-1", getFontHeight(display, th) - 4);

	Font f = XLoadFont(display, fontName);

	std::string temp = " *** " + what + " *** ";

	// holder for a text
	XTextItem xti;
	xti.chars = (char *)temp.c_str();
	xti.nchars = strlen(xti.chars);
	xti.delta = 0;
	xti.font = f;

	// determine width in pixels of a string
	XFontStruct *fs = XQueryFont(display, f);
	int s = XTextWidth(fs, xti.chars, xti.nchars);

	// create an image structure
	int depth = DefaultDepth(display, DefaultScreen(display));
	Pixmap pixmap = XCreatePixmap(display, window, s, th, depth);

	XSetForeground(display, gc, BlackPixelOfScreen(DefaultScreenOfDisplay(display)));
	XSetFillStyle(display, gc, FillSolid);
	XFillRectangle(display, pixmap, gc, 0, 0, s, th);

	// printf("descent: %d\n", fs -> descent);
	// -2: tiny border
	XSetForeground(display, gc, WhitePixelOfScreen(DefaultScreenOfDisplay(display)));

	XDrawText(display, pixmap, gc, 0, th - fs -> descent - 2, &xti, 1);

	XUnloadFont(display, f);

	int x = 0;
	for(;;) {
		XCopyArea(display, pixmap, window, gc, x, 0, std::min(width, s - x), th, 0, height / 8);
		XFlush(display);

		x += 5;
		if (x >= s)
			 break;

		usleep(5000);
	}

	XFreePixmap(display, pixmap);
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("connected to mqtt, rc: %d\n", result);
}

volatile bool got_msg = false;
std::string msg;

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	bool match = 0;
	printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);

	if (strcmp(message->topic, mqtt_topic.c_str()) == 0) {
		got_msg = true;
		msg = std::string((const char *)message -> payload, message -> payloadlen);
	}
}

int main(int argc, char **argv)
{
	printf("ntj-mqtt is (C) 2017 by Folkert van Heusden\n");
	printf("Released under AGPL v3.0\n");

	mosquitto_lib_init();

	char clientid[24];
	snprintf(clientid, sizeof clientid, "ntj-mqtt_%d", getpid());

	struct mosquitto *mosq = mosquitto_new(clientid, true, 0);

	mosquitto_connect_callback_set(mosq, connect_callback);
	mosquitto_message_callback_set(mosq, message_callback);

	mosquitto_connect(mosq, mqtt_host.c_str(), mqtt_port, 60);
	mosquitto_subscribe(mosq, NULL, mqtt_topic.c_str(), 0);

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

	for(;;)
	{
		double re = drand48() * 6.0 - 3.0;
		double im = drand48() * 6.0 - 3.0;

		//printf("%f,%f %f,%f\n", pre, pim, re, im);
		double add_re = re - pre;
		double add_im = im - pim;

		double start_ts = get_ts(), end_ts = start_ts + duration, now_ts = start_ts;
		int frames = 0;

		if (mosquitto_loop(mosq, 1, 1) != MOSQ_ERR_SUCCESS)
			mosquitto_reconnect(mosq);

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

			if (use_it < 1)
				use_it = 1;
			else if (use_it > 100)
				use_it = 100;

			if (fps > target_fps)
				usleep(1000);

			if (got_msg) {
				got_msg = false;
				show_text(display, window, width, height, gc, msg);
			}
		}

		pre = re;
		pim = im;
	}

	return 0;
}
