///
/// Basic X11 + OpenGL usage
///
/// Adapted from:
/// https://www.khronos.org/opengl/wiki/Programming_OpenGL_in_Linux:_GLX_and_Xlib
///
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#define UNUSED __attribute__((unused))

#define eprintf(...) (void)fprintf(stderr, __VA_ARGS__)

static GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

static void print_gl_version(void)
{
	int major = 0;
	int minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("OpenGL Version: %d.%d\n", major, minor);
}

static void print_glew_version(void)
{
	GLubyte const *version = glewGetString(GLEW_VERSION);
	printf("GLEW Version: %s\n", version);
}

void GLAPIENTRY message_callback(
	UNUSED GLenum      source,
	GLenum             type,
	UNUSED GLuint      id,
	GLenum             severity,
	UNUSED GLsizei     length,
	GLchar const      *message,
	UNUSED void const *user)
{
	eprintf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type,
		severity,
		message);
}

static void draw_quad(void)
{
	glClearColor(1.0F, 1.0F, 1.0F, 1.0F);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, 20.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glBegin(GL_QUADS);
	glColor3f(1.0F, 0.0F, 0.0F);
	glVertex3f(-0.75F, -0.75F, 0.0F);
	glColor3f(0.0F, 1.0F, 0.0F);
	glVertex3f(0.75F, -0.75F, 0.0F);
	glColor3f(0.0F, 0.0F, 1.0F);
	glVertex3f(0.75F, 0.75F, 0.0F);
	glColor3f(1.0F, 1.0F, 0.0F);
	glVertex3f(-0.75F, 0.75F, 0.0F);
	glEnd();
}

int main(void)
{
	int ret = EXIT_FAILURE;

	Display *dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		eprintf("Failed to connect to X server\n");
		return EXIT_FAILURE;
	}

	Window root = DefaultRootWindow(dpy);

	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		eprintf("No visual found\n");
		goto out_close_display;
	}

	unsigned long valuemask = CWColormap | CWEventMask;

	XSetWindowAttributes swa = {
		.colormap   = XCreateColormap(dpy, root, vi->visual, AllocNone),
		.event_mask = ExposureMask | KeyPressMask,
	};

	Window win = XCreateWindow(
		dpy,         // display
		root,        // parent
		0,           // x
		0,           // y
		600,         // width
		600,         // height
		0,           // border_width
		vi->depth,   // depth
		InputOutput, // class
		vi->visual,  // visual
		valuemask,   // valuemask
		&swa         // attributes
	);

	XClassHint *class_hint = XAllocClassHint();
	if (class_hint == NULL) {
		eprintf("Failed to allocate class hint\n");
		goto out_destroy_window;
	}

	class_hint->res_class = "c_bits_window";
	XSetClassHint(dpy, win, class_hint);

	Atom wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	int  rc               = XSetWMProtocols(dpy, win, &wm_delete_window, 1);
	if (rc != 1) {
		eprintf("Failed to XSetWMProtocols\n");
		goto out_free_class_hint;
	}

	XMapWindow(dpy, win);
	XStoreName(dpy, win, "c_bits_window");

	GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	if (glc == NULL) {
		eprintf("Failed to create OpenGL context\n");
		goto out_free_class_hint;
	}

	glXMakeCurrent(dpy, win, glc);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		eprintf("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
		goto out_destroy_context;
	}

	print_gl_version();
	print_glew_version();

	glEnable(GL_DEPTH_TEST);

	if (GLEW_ARB_debug_output) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(message_callback, NULL);
		printf("Enabled debug output\n");
	}

	XEvent            xev;
	XWindowAttributes gwa;

	int loop_stat = 1;
	while (loop_stat == 1) {
		XNextEvent(dpy, &xev);
		switch (xev.type) {
		case Expose:
			XGetWindowAttributes(dpy, win, &gwa);
			glViewport(0, 0, gwa.width, gwa.height);
			draw_quad();
			glXSwapBuffers(dpy, win);
			break;
		case ClientMessage:
			if (xev.xclient.data.l[0] == (long)wm_delete_window) {
				printf("Received WM_DELETE_WINDOW\n");
				loop_stat = 0;
			}
			break;
		default:
			break;
		}
	}

	ret = EXIT_SUCCESS;
	glXMakeCurrent(dpy, None, NULL);
out_destroy_context:
	glXDestroyContext(dpy, glc);
out_free_class_hint:
	XFree(class_hint);
out_destroy_window:
	XDestroyWindow(dpy, win);
	XFree(vi);
out_close_display:
	XCloseDisplay(dpy);
	return ret;
}
