//+++HDR+++
//======================================================================
//   This file is part of the RATMAN software framework.
//   Copyright (C) 2009 by CRS4, Pula, Italy.
//
//   For more information, visit the CRS4 Visual Computing Group
//   web pages at http://www.crs4.it/vic/
//
//   This file may be used under the terms of the GNU General Public
//   License as published by the Free Software Foundation and appearing
//   in the file LICENSE included in the packaging of this file.
//
//   CRS4 reserves all rights not expressly granted herein.
//  
//   This file is provided AS IS with NO WARRANTY OF ANY KIND, 
//   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS 
//   FOR A PARTICULAR PURPOSE.
//
//======================================================================
//---HDR---//
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <vector>
#include <iostream>

#include <vic/gl/font.hpp>

static vic::gl::font* proportional_glfont = 0;
static vic::gl::font* fixed_glfont = 0;

static void draw_frame(void) {
  glClearColor(0.18, 0.18, 0.36, 0.0);
  //glClearColor(1.0,1.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH); 
    glDisable( GL_CULL_FACE );    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);  

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho(0.0f, 800.0f,
            0.0f, 450.0f,
            -1.0f,
            1.0f);

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    if (!proportional_glfont) {
      proportional_glfont = new vic::gl::font();
      proportional_glfont->set_proportional(true);
    }
    if (!fixed_glfont) {
      fixed_glfont = new vic::gl::font();
      fixed_glfont->set_proportional(false);
    }
    
    glPushMatrix(); {
      glTranslatef(24.0f, 24.0f, 0.0f);
      glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
      proportional_glfont->write("This is a proportional test string");

      glTranslatef(0.0f, 32.0f, 0.0f);
      glScalef(0.5f,0.5f,0.0f);
      glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
      proportional_glfont->write("This is a scaled proportional test string");

      glTranslatef(0.0f, 32.0f, 0.0f);
      glScalef(0.5f,0.5f,0.0f);
      glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
      proportional_glfont->write("This is a scaled proportional test string");
    }
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
}


/* new window size or exposure */
static void
reshape(int width, int height) {
   GLfloat h = (GLfloat) height / (GLfloat) width;

   glViewport(0, 0, (GLint) width, (GLint) height);
}


static void init(void) {
}


/*
 * Create an RGB, double-buffered window.
 * Return the window and context handles.
 */
static void make_window( Display *dpy, const char *name,
			 int x, int y, int width, int height,
			 Window *winRet, GLXContext *ctxRet) {
   int attrib[] = { GLX_RGBA,
		    GLX_RED_SIZE, 1,
		    GLX_GREEN_SIZE, 1,
		    GLX_BLUE_SIZE, 1,
		    GLX_DOUBLEBUFFER,
		    GLX_DEPTH_SIZE, 1,
		    None };
   int scrnum;
   XSetWindowAttributes attr;
   unsigned long mask;
   Window root;
   Window win;
   GLXContext ctx;
   XVisualInfo *visinfo;

   scrnum = DefaultScreen( dpy );
   root = RootWindow( dpy, scrnum );

   visinfo = glXChooseVisual( dpy, scrnum, attrib );
   if (!visinfo) {
      printf("Error: couldn't get an RGB, Double-buffered visual\n");
      exit(1);
   }

   /* window attributes */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap( dpy, root, visinfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

   win = XCreateWindow( dpy, root, 0, 0, width, height,
		        0, visinfo->depth, InputOutput,
		        visinfo->visual, mask, &attr );

   /* set hints and properties */
   {
      XSizeHints sizehints;
      sizehints.x = x;
      sizehints.y = y;
      sizehints.width  = width;
      sizehints.height = height;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(dpy, win, &sizehints);
      XSetStandardProperties(dpy, win, name, name,
                              None, (char **)NULL, 0, &sizehints);
   }

   ctx = glXCreateContext( dpy, visinfo, NULL, True );
   if (!ctx) {
      printf("Error: glXCreateContext failed\n");
      exit(1);
   }

   XFree(visinfo);

   *winRet = win;
   *ctxRet = ctx;
}


static void event_loop(Display *dpy, Window win) {
   while (1) {
      while (XPending(dpy) > 0) {
         XEvent event;
         XNextEvent(dpy, &event);
         switch (event.type) {
	 case Expose:
            /* we'll redraw below */
	    break;
	 case ConfigureNotify:
	    reshape(event.xconfigure.width, event.xconfigure.height);
	    break;
         case KeyPress:
            {
               char buffer[10];
	       int r = XLookupString(&event.xkey, buffer, sizeof(buffer),
				 NULL, NULL);
	       if (buffer[0] == 27) {
		   /* escape */
		   return;
	       }
            }
         }
      }

      draw_frame();
      glXSwapBuffers(dpy, win);
   }
}


int
main(int argc, char *argv[])
{
   Display *dpy;
   Window win;
   GLXContext ctx;
   char *dpyName = NULL;
   GLboolean printInfo = GL_FALSE;
   int i;

   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-display") == 0) {
         dpyName = argv[i+1];
         i++;
      }
      else if (strcmp(argv[i], "-info") == 0) {
         printInfo = GL_TRUE;
      }
   }

   dpy = XOpenDisplay(dpyName);
   if (!dpy) {
      printf("Error: couldn't open display %s\n", dpyName);
      return -1;
   }

   make_window(dpy, "glxgears", 0, 0, 800, 450, &win, &ctx);
   XMapWindow(dpy, win);
   glXMakeCurrent(dpy, win, ctx);
   reshape(300, 300);

   if (printInfo) {
      printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
      printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
      printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
      printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
   }

   init();

   event_loop(dpy, win);

   glXDestroyContext(dpy, ctx);
   XDestroyWindow(dpy, win);
   XCloseDisplay(dpy);

   return 0;
}
