#include "environment_map.h"
#include "ijksdl/ijksdl_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <math.h>

#define PI						3.14159265358979323846
#define PI_2					1.57079632679489661923   // pi / 2
#define PI2						6.28318530717958647692   // pi * 2

#define N_STACKS				90
#define N_SLICES				180
#define N_VERTEX_COUNT			( ((N_STACKS) + 1) * ((N_SLICES) + 1) )
#define N_INDEX_COUNT			( ((N_STACKS) - 1) * (N_SLICES) * 6 )

#define IDX_VERTEX_BUFFER		0
#define IDX_INDEX_BUFFER		1

static void spherical_get_positions(int stacks, int slices, GLfloat* positions)
{
	int i;
	int stack;
	int slice;
	double x, y, z;
	double phi, theta;

	i = 0;

	// right hander coordination
	for (stack = 0; stack <= stacks; stack++)
	{
		phi = PI_2 - stack * PI / stacks;
		y = sin(phi);

		for (slice = 0; slice <= slices; slice++)
		{
			theta = PI2 * slice / slices;
			x = cos(phi) * sin(theta);
			z = cos(phi) * cos(theta);

			positions[i++] = (GLfloat)x;
			positions[i++] = (GLfloat)y;
			positions[i++] = (GLfloat)z;
			positions[i++] = 1.0f * slice / slices; // u-texture coordinate
			positions[i++] = 1.0f * stack / stacks; // v-texture coordinate
		}
	}

	assert(i == (stacks + 1) * (slices + 1) * 5);
}

static void spherical_get_indices(int stacks, int slices, GLushort* indices)
{
	int i, top, bot;
	int stack;
	int slice;

	i = 0;

	// right hander coordination
	for (stack = 0; stack < stacks; stack++)
	{
		top = stack * (slices + 1);
		bot = (stack + 1) * (slices + 1);

		for (slice = 0; slice < slices; slice++)
		{
			if (0 != stack)
			{
				indices[i++] = (GLushort)(top + slice);
				indices[i++] = (GLushort)(bot + slice);
				indices[i++] = (GLushort)(top + slice + 1);
			}

			if (stack != stacks - 1)
			{
				indices[i++] = (GLushort)(top + slice + 1);
				indices[i++] = (GLushort)(bot + slice);
				indices[i++] = (GLushort)(bot + slice + 1);
			}
		}
	}

	assert(i == (stacks - 1) * slices * 6);
}

typedef struct _spherical_map_t
{
	int width, height;
	//GLuint		av4_position;
	//GLuint		av2_texture;
	//GLfloat		vertex[N_VERTEX_COUNT * 5]; // x/y/z + u/v per vertex
	//GLfloat		index[N_INDEX_COUNT];
	GLuint		buffer[2];
} spherical_map_t;

static GLfloat s_vertex[N_VERTEX_COUNT * 5]; // x/y/z + u/v per vertex
static GLushort s_index[N_INDEX_COUNT];

static void spherical_init()
{
	spherical_get_positions(N_STACKS, N_SLICES, s_vertex);
	spherical_get_indices(N_STACKS, N_SLICES, s_index);
}

static void* spherical_create()
{
	spherical_map_t* sphere;

	sphere = (spherical_map_t*)malloc(sizeof(spherical_map_t));
	if (sphere)
	{
		memset(sphere, 0, sizeof(spherical_map_t));

		glGenBuffers(sizeof(sphere->buffer)/sizeof(sphere->buffer[0]), sphere->buffer);
		if (0 != glGetError())
		{
			free(sphere);
			return NULL;
		}

		glBindBuffer(GL_ARRAY_BUFFER, sphere->buffer[IDX_VERTEX_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, N_VERTEX_COUNT * 5 * sizeof(GLfloat), s_vertex, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->buffer[IDX_INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, N_INDEX_COUNT * sizeof(GLushort), s_index, GL_STATIC_DRAW);
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

static void spherical_setup(void* proj, GLuint v4Position, GLuint v2Texture)
{
	spherical_map_t* sphere;
	sphere = (spherical_map_t*)proj;
	//sphere->av2_texture = v2Texture;
	//sphere->av4_position = v4Position;

	glBindBuffer(GL_ARRAY_BUFFER, sphere->buffer[IDX_VERTEX_BUFFER]);
	glVertexAttribPointer(v4Position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)0);
	glEnableVertexAttribArray(v4Position);
	glVertexAttribPointer(v2Texture, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(v2Texture);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void IJK_GLES2_Matrix_Ortho(GLfloat *matrix, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
void IJK_GLES2_Matrix_MultiplyMM(GLfloat r[16], const GLfloat lhs[16], const GLfloat rhs[16]);

static void spherical_draw(void* proj, GLsizei width, GLsizei height, GLuint mat4MVP, const GLfloat viewMatrix[16])
{
	spherical_map_t* sphere;	
	GLfloat scaleMatrix[16];
	GLfloat projMatrix[16];
	GLfloat mvpMatix[16];

	sphere = (spherical_map_t*)proj;
	if (0 != width)
		sphere->width = width;
	if (0 != height)
		sphere->height = height;

	memset(scaleMatrix, 0, sizeof(scaleMatrix));
	scaleMatrix[0] = 2.0f;
	scaleMatrix[5] = 2.0f;
	scaleMatrix[15] = 1.0f;
	IJK_GLES2_Matrix_Ortho(projMatrix, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	IJK_GLES2_Matrix_MultiplyMM(mvpMatix, projMatrix, viewMatrix);
	IJK_GLES2_Matrix_MultiplyMM(projMatrix, scaleMatrix, mvpMatix);

	// left eye
	glViewport(0, 0, width / 2, height);
	glUniformMatrix4fv(mat4MVP, 1, GL_FALSE, projMatrix);
	IJK_GLES2_checkError_TRACE("glUniformMatrix4fv(um4_mvp)");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->buffer[IDX_INDEX_BUFFER]);
	glDrawElements(GL_TRIANGLES, N_INDEX_COUNT, GL_UNSIGNED_SHORT, (const void*)0); 
	IJK_GLES2_checkError_TRACE("glDrawElements");

	// right eye
	glViewport(width / 2, 0, width / 2, height);
	//glUniformMatrix4fv(mat4MVP, 1, GL_FALSE, mvpMatrix);
	//IJK_GLES2_checkError_TRACE("glUniformMatrix4fv(um4_mvp)");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->buffer[IDX_INDEX_BUFFER]);
	glDrawElements(GL_TRIANGLES, N_INDEX_COUNT, GL_UNSIGNED_SHORT, (const void*)0);
	IJK_GLES2_checkError_TRACE("glDrawElements");
}

environment_map_t* environment_map_shperical()
{
	static environment_map_t s_map = 
	{
		spherical_create,
		spherical_destroy,
		spherical_setup,
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
