#include "opengl-shader.h"
#include <stdio.h>
#include <stdlib.h>

static GLuint opengl_load_shader(GLenum type, const char* string)
{
	GLuint shader;
	GLint errcode;

	shader = glCreateShader(type);

	glShaderSource(shader, 1, &string, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &errcode);
	if (GL_FALSE == errcode)
	{
		char msg[512];
		glGetShaderInfoLog(shader, 512, &errcode, msg);
		printf("[OPENGL] load shader failed: %s\n%s", msg, string);
		glDeleteShader(shader);
		shader = 0;
	}

	return shader;
}

static GLuint opengl_load_program(GLuint vertex, GLuint fragment)
{
	GLuint program;
	GLint errcode;

	program = glCreateProgram();

	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &errcode);
	if (GL_FALSE == errcode)
	{
		char msg[512];
		glGetProgramInfoLog(program, 512, &errcode, msg);
		printf("[OPENGL] load program failed: %s\n", msg);
		glDeleteProgram(program);
		program = 0;
	}
	return program;
}

int opengl_shader_create(struct opengl_shader_t* shader, const char* vertex, const char* fragment)
{
	shader->vertex = opengl_load_shader(GL_VERTEX_SHADER, vertex);
	shader->fragment = opengl_load_shader(GL_FRAGMENT_SHADER, fragment);
	if (0 == shader->vertex || 0 == shader->fragment)
		return -1;

	shader->program = opengl_load_program(shader->vertex, shader->fragment);

	//glDeleteShader(shader->vertex);	// flagged for deletion
	//glDeleteShader(shader->fragment);	// flagged for deletion
	return 0 == shader->program ? -1 : 0;
}

int opengl_shader_destroy(struct opengl_shader_t* shader)
{
	if (!shader)
		return -1;

	if (shader->program)
	{
		glDeleteProgram(shader->program);
	}

	if (shader->vertex)
	{
		glDeleteShader(shader->vertex);
	}

	if (shader->fragment)
	{
		glDeleteShader(shader->fragment);
	}

	memset(shader, 0, sizeof(struct opengl_shader_t));
	return 0;
}
