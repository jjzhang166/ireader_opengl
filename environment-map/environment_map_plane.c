#include "environment_map.h"
#include <stdlib.h>
#include <assert.h>

static GLfloat s_vertex[] = {
	-1.0f, 1.0f,
	-1.0f, -1.0f,
	1.0f, 1.0f,
	1.0f, -1.0f,
};

static GLfloat s_texture[] = {
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
};

static void* plane_create()
{
	return (void*)1;
}

static void plane_destroy(void* p)
{
	(void)p;
}

static void plane_setup(void* p, GLuint v4Position, GLuint v2Texture)
{
	glVertexAttribPointer(v4Position, 2, GL_FLOAT, GL_FALSE, 0, s_vertex);
	glEnableVertexAttribArray(v4Position);
	glVertexAttribPointer(v2Texture, 2, GL_FLOAT, GL_FALSE, 0, s_texture);
	glEnableVertexAttribArray(v2Texture);
}

void opengl_matrix_ortho(GLfloat *matrix, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
static void plane_draw(void* p, GLuint mat4MVP, const GLfloat viewMatrix[16])
{
	(void)p;
	static GLfloat projMatrix[16] = { 0 };
	opengl_matrix_ortho(projMatrix, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

//	glViewport(0, 0, width, height); // setting in IJK_EGL_prepareRenderer

	glUniformMatrix4fv(mat4MVP, 1, GL_FALSE, projMatrix);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

environment_map_t* environment_map_plane()
{
	static environment_map_t s_map = {
		plane_create,
		plane_destroy,
		plane_setup,
		plane_draw,
	};

	return &s_map;
}
