#include "environment_map.h"
#include <stdlib.h>
#include <assert.h>

static GLfloat s_vertex[] = {
	-1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
};

static void* plane_create()
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * 5 * sizeof(GLfloat), s_vertex, GL_STATIC_DRAW);

	return (void*)buffer;
}

static void plane_destroy(void* p)
{
	GLuint buffer;
	buffer = (GLuint)p;
	glDeleteBuffers(1, &buffer);
}

static void plane_setup(void* p, GLuint v4Position, GLuint v2Texture)
{
	GLuint buffer;
	buffer = (GLuint)p;

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glVertexAttribPointer(v4Position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)0);
	glEnableVertexAttribArray(v4Position);
	glVertexAttribPointer(v2Texture, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(v2Texture);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void IJK_GLES2_Matrix_Ortho(GLfloat *matrix, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
static void plane_draw(void* p, GLsizei width, GLsizei height, GLuint mat4MVP, const GLfloat viewMatrix[16])
{
	(void)p;
	GLfloat projMatrix[16] = { 0 };
	IJK_GLES2_Matrix_Ortho(projMatrix, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

	//GLfloat mvpMatix[16];
	//IJK_GLES2_Matrix_MultiplyMM(mvpMatix, projMatrix, viewMatrix);

	glViewport(0, 0, width, height);

	glUniformMatrix4fv(mat4MVP, 1, GL_FALSE, projMatrix);
	IJK_GLES2_checkError_TRACE("glUniformMatrix4fv(um4_mvp)");

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	IJK_GLES2_checkError_TRACE("glDrawArrays(GL_TRIANGLE_STRIP) plane");
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
