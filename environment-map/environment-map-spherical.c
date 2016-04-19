#include "environment-map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <math.h>

#define N_STACKS				180
#define N_SLICES				360
#define N_VERTEX_COUNT			( ((N_STACKS) + 1) * ((N_SLICES) + 1) )
#define N_INDEX_COUNT			( ((N_STACKS) - 1) * (N_SLICES) * 6 )

#define IDX_VERTEX_BUFFER		0
#define IDX_INDEX_BUFFER		1

#define INTERPUPILLARY_DISTANCE	0.06f
#define LENS_DIAMETER			0.025f	// 25MM

void opengl_matrix_ortho(GLfloat m[16], GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
void opengl_matrix_frustum(GLfloat m[16], GLfloat left, GLfloat right, GLfloat top, GLfloat bottom, GLfloat near, GLfloat far);
void opengl_matrix_perspective(GLfloat m[16], GLfloat fovy, GLfloat aspect, GLfloat near, GLfloat far);
void opengl_matrix_multiply_mm(GLfloat m[16], const GLfloat lhs[16], const GLfloat rhs[16]);
void opengl_matrix_scale(GLfloat m[16], GLfloat x, GLfloat y, GLfloat z);
void opengl_matrix_translate(GLfloat m[16], GLfloat x, GLfloat y, GLfloat z);

typedef struct _spherical_map_t
{
	//GLfloat		vertex[N_VERTEX_COUNT * 5]; // x/y/z + u/v per vertex
	//GLfloat		index[N_INDEX_COUNT];
	int			vrmode;
	GLuint		buffer[2];
} spherical_map_t;

static GLfloat s_vertex[N_VERTEX_COUNT * 5]; // x/y/z + u/v per vertex
static GLuint s_index[N_INDEX_COUNT];

static void spherical_draw(void* proj, GLuint v4Position, GLuint v2Texture, GLuint mat4MVP, const GLfloat viewMatrix[16])
{
	spherical_map_t* sphere;
	GLfloat translateMatrix[16];
	GLfloat projMatrix[16];
	GLfloat mvpMatix[16];
	GLint viewport[4];
	GLint frontface[1];

	sphere = (spherical_map_t*)proj;
	glGetIntegerv(GL_FRONT_FACE, frontface);
//	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	glBindBuffer(GL_ARRAY_BUFFER, sphere->buffer[IDX_VERTEX_BUFFER]);
	glVertexAttribPointer(v4Position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)0);
	glEnableVertexAttribArray(v4Position);
	glVertexAttribPointer(v2Texture, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(v2Texture);

	glGetIntegerv(GL_VIEWPORT, viewport);
	opengl_matrix_perspective(projMatrix, 90.0f, 1.0f * viewport[2] / viewport[3] / (2 - sphere->vrmode), 0.01, 10.0f);
//	opengl_matrix_ortho(projMatrix, -0.7071f, 0.7071f, -0.7071f, 0.7071f, 1.0f, 0.0f);
//	opengl_matrix_ortho(projMatrix, -1.0f * ration, 1.0f * ration, -1.0f, 1.0f, 1.0f, -1.0f);
	opengl_matrix_multiply_mm(mvpMatix, projMatrix, viewMatrix);

	// left eye
	glViewport(viewport[0], viewport[1], viewport[2] / (2 - sphere->vrmode), viewport[3]);
	opengl_matrix_translate(translateMatrix, INTERPUPILLARY_DISTANCE/2, 0.0f, 0.0f);
	opengl_matrix_multiply_mm(projMatrix, translateMatrix, mvpMatix);
	glUniformMatrix4fv(mat4MVP, 1, GL_FALSE, projMatrix);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->buffer[IDX_INDEX_BUFFER]);
	glDrawElements(GL_TRIANGLES, N_INDEX_COUNT, GL_UNSIGNED_INT, (const void*)0);

	// right eye
	if (0 == sphere->vrmode)
	{
		glViewport(viewport[0] + viewport[2] / 2, viewport[1], viewport[2] / 2, viewport[3]);
		opengl_matrix_translate(translateMatrix, -INTERPUPILLARY_DISTANCE / 2, 0.0f, 0.0f);
		opengl_matrix_multiply_mm(projMatrix, translateMatrix, mvpMatix);
		glUniformMatrix4fv(mat4MVP, 1, GL_FALSE, projMatrix);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->buffer[IDX_INDEX_BUFFER]);
		glDrawElements(GL_TRIANGLES, N_INDEX_COUNT, GL_UNSIGNED_INT, (const void*)0);
	}

	glFrontFace(frontface[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(v4Position);
	glDisableVertexAttribArray(v2Texture);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

static void* spherical_create()
{
	spherical_map_t* sphere;

	sphere = (spherical_map_t*)malloc(sizeof(spherical_map_t));
	if (sphere)
	{
		memset(sphere, 0, sizeof(spherical_map_t));

		glGenBuffers(sizeof(sphere->buffer) / sizeof(sphere->buffer[0]), sphere->buffer);
		if (0 != glGetError())
		{
			free(sphere);
			return NULL;
		}

		glBindBuffer(GL_ARRAY_BUFFER, sphere->buffer[IDX_VERTEX_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, N_VERTEX_COUNT * 5 * sizeof(GLfloat), s_vertex, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->buffer[IDX_INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, N_INDEX_COUNT * sizeof(GLuint), s_index, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	return sphere;
}

static void spherical_destroy(void* p)
{
	spherical_map_t* sphere;
	if (NULL == p)
		return;

	sphere = (spherical_map_t*)p;
	glDeleteBuffers(sizeof(sphere->buffer) / sizeof(sphere->buffer[0]), sphere->buffer);
	memset(sphere, 0, sizeof(spherical_map_t));
	free(sphere);
}

void spherical_set_vrmode(void* p, unsigned int mode)
{
	spherical_map_t* sphere;
	if (NULL == p)
		return;

	sphere = (spherical_map_t*)p;
	sphere->vrmode = mode % 2;
}

GLsizei sphere_vertex_count(int stacks, int slices);
GLsizei sphere_index_count(int stacks, int slices);
void sphere_vertex(int stacks, int slices, GLfloat* positions);
void sphere_index(int stacks, int slices, GLuint* indices);

static void spherical_init()
{
	assert(sphere_vertex_count(N_STACKS, N_SLICES) * 5 == sizeof(s_vertex) / sizeof(s_vertex[0]));
	assert(sphere_index_count(N_STACKS, N_SLICES) == sizeof(s_index) / sizeof(s_index[0]));
	sphere_vertex(N_STACKS, N_SLICES, s_vertex);
	sphere_index(N_STACKS, N_SLICES, s_index);
}

environment_map_t* environment_map_shperical()
{
	static environment_map_t s_map = 
	{
		spherical_create,
		spherical_destroy,
		spherical_draw,
	};

	static int i = 0;
	if (0 == i)
	{
		i = 1;
		spherical_init();
	}

	return &s_map;
}
