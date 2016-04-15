#ifndef _opengl_es2_h_
#define _opengl_es2_h_

#if defined(OS_WINDOWS) || defined(_WIN32) || defined(_WIN64)
#include <gl/GL.h>
#elif defined(__APPLE__)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#endif

#endif /* !_opengl_es2_h_ */
