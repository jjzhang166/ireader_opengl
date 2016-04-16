#ifndef _opengl_shader_h_
#define _opengl_shader_h_

#include "opengl-es2.h"

struct opengl_shader_t
{
	GLuint vertex;
	GLuint fragment;
	GLuint program;
};

int opengl_shader_create(struct opengl_shader_t* shader, const char* vertex, const char* fragment);

int opengl_shader_destroy(struct opengl_shader_t* shader);

#endif /* !_opengl_shader_h_ */
