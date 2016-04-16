#ifndef _opengl_fbo_h_
#define _opengl_fbo_h_

#include "opengl-es2.h"

struct opengl_fbo_t
{
	GLuint framebuffer;
	GLuint renderbuffer;
	GLuint texture;
	GLsizei width;
	GLsizei height;
};

int opengl_fbo_create(struct opengl_fbo_t* fbo, int width, int height);

int opengl_fbo_destroy(struct opengl_fbo_t* fbo);

#endif /* !_opengl_fbo_h_ */
