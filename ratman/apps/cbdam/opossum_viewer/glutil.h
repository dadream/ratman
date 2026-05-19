#ifndef GLUTIL_H_
#define GLUTIL_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

  extern void glutil_begin_screen_coordinates_system();

  extern void glutil_end_screen_coordinates_system();

  /**
   *  Format and render a message over a GL
   *  buffer. x,y, and h are in screen coordinates
   *  system. (0,0) is the lower left corner,
   *  1 means 1 pixel.
   */
  extern void glutil_printf(const GLfloat x,
                            const GLfloat y,
                            const GLfloat h,
                            const char *fmt,
                            ...);

  /**
   *  Equivalent to glutil_printf, except that it is
   *  called with a va_list  instead of a variable
   *  number of arguments. The
   *  function does not call the va_end macro.
   *  Consequently,  the value  of  ap is undefined
   *  after the call. The application
   *  should call va_end(ap) itself afterwards.
   */
  extern void glutil_vprintf(const GLfloat x,
                             const GLfloat y,
                             const GLfloat h,
                             const char *fmt,
                             va_list ap);
    
#ifdef __cplusplus
}
#endif

#endif
