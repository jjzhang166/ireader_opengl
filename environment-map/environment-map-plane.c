#include "environment-map.h"
#include <stdlib.h>
#include <assert.h>

void opengl_matrix_ortho(GLfloat *matrix, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);

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

static void plane_draw(void* p, GLuint v4Position, GLuint uViewMatrix, const GLfloat viewMatrix[16], GLuint v2Texture, GLuint uTextureMatrix, const GLfloat textureMatrix[16])
{
	(void)p;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	glViewport(0, 0, width, height); // setting in IJK_EGL_prepareRenderer
//	glEnable(GL_CULL_FACE);
//	glEnable(GL_DEPTH_TEST);
//	glFrontFace(GL_CCW);

//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribPointer(v4Position, 2, GL_FLOAT, GL_FALSE, 0, s_vertex);
	glEnableVertexAttribArray(v4Position);
	glVertexAttribPointer(v2Texture, 2, GL_FLOAT, GL_FALSE, 0, s_texture);
	glEnableVertexAttribArray(v2Texture);

	glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(uTextureMatrix, 1, GL_FALSE, textureMatrix);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(v4Position);
	glDisableVertexAttribArray(v2Texture);
}

static void* plane_create()
{
	return (void*)1;
}

static void plane_destroy(void* p)
{
	(void)p;
}

environment_map_t* environment_map_plane()
{
	static environment_map_t s_map = {
		plane_create,
		plane_destroy,
		plane_draw,
	};

	return &s_map;
}
