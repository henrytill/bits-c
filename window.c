#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>

static GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

static void draw_quad(void) {
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

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[]) {
  int rc = 0;

  Display *dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    rc = fprintf(stderr, "Failed to connect to X server\n");
    assert(rc > 0);
    return EXIT_FAILURE;
  }

  Window root = DefaultRootWindow(dpy);

  XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
  if (vi == NULL) {
    rc = fprintf(stderr, "No visual found\n");
    assert(rc > 0);
    XCloseDisplay(dpy);
    return EXIT_FAILURE;
  }

  printf("visual: %p", (void *)vi->visualid);

  Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

  XSetWindowAttributes swa = {
    .colormap = cmap,
    .event_mask = ExposureMask | KeyPressMask,
  };

  Window win = XCreateWindow(dpy, root, 0, 0, 600, 600, 0,
                             vi->depth, InputOutput,
                             vi->visual, CWColormap | CWEventMask,
                             &swa);

  XMapWindow(dpy, win);
  XStoreName(dpy, win, "VERY SIMPLE APPLICATION");

  GLXContext glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
  glXMakeCurrent(dpy, win, glc);

  glEnable(GL_DEPTH_TEST);

  XEvent xev;
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
    case KeyPress:
      glXMakeCurrent(dpy, None, NULL);
      loop_stat = 0;
      break;
    }
  }

  glXDestroyContext(dpy, glc);
  free(vi);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
  return EXIT_SUCCESS;
}
