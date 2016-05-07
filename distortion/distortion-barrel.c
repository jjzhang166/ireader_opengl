#include "distortion.h"
#include "opengl-fbo.h"
#include "opengl-shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#define STRINGZ(x)	#x
#define STRINGZ2(x)	STRINGZ(x)
#define STRING(x)	STRINGZ2(x)

#define ROW 40
#define COL 40

#define BARREL_COEFFICIENT_K0 0.005		//250.0
#define BARREL_COEFFICIENT_K1 0.1		//50000.0

#define ABS(x) ((x) < 0) ? (-1 * (x)) : (x)

static GLushort s_index[ROW * COL * 6];
static GLfloat s_vertex[(ROW + 1) * (COL + 1) * 4];

typedef struct _barrel_distortion_t
{
	struct opengl_fbo_t fbo;
	struct opengl_shader_t shader;

	GLuint framebuffer0;
	GLuint buffer[2];

	GLint pid; // attribute v_position
	GLint tid; // attribute v_texture
	GLint uid; // uniform sampler2D tex0
} barrel_distortion_t;

static void barrel_distortion_after(void* distortion)
{
	GLint active[1];
	GLint program[1];
	GLint texture[1];
	GLint viewport[4];
	GLint arraybuffer[1];
	GLint elementarraybuffer[1];
	barrel_distortion_t* barrel;
	barrel = (barrel_distortion_t*)distortion;
	assert(barrel->shader.program);

	if (0 == barrel->fbo.framebuffer)
		return;

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetIntegerv(GL_ACTIVE_TEXTURE, active);
	glGetIntegerv(GL_CURRENT_PROGRAM, program);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, arraybuffer);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, elementarraybuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, barrel->framebuffer0);
	glUseProgram(barrel->shader.program);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);

	glBindBuffer(GL_ARRAY_BUFFER, barrel->buffer[0]);
	glVertexAttribPointer(barrel->pid, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (const void*)0);
	glEnableVertexAttribArray(barrel->pid);
	glVertexAttribPointer(barrel->tid, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (const void*)(2 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(barrel->tid);

	glActiveTexture(GL_TEXTURE0);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, texture);
	glBindTexture(GL_TEXTURE_2D, barrel->fbo.texture);
	glUniform1i(barrel->uid, 0);

	//glEnable(GL_SCISSOR_TEST);
	glViewport(viewport[0], viewport[1], viewport[2] / 2, viewport[3]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, barrel->buffer[1]);
	glDrawElements(GL_TRIANGLES, sizeof(s_index) / sizeof(s_index[0]), GL_UNSIGNED_SHORT, 0);

	glViewport(viewport[0] + viewport[2] / 2, viewport[1], viewport[2] / 2, viewport[3]);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, barrel->buffer[1]);
	glDrawElements(GL_TRIANGLES, sizeof(s_index) / sizeof(s_index[0]), GL_UNSIGNED_SHORT, 0);

	// restore opengl context
	glDisableVertexAttribArray(barrel->pid);
	glDisableVertexAttribArray(barrel->tid);

	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	glUseProgram(program[0]);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementarraybuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, arraybuffer[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glActiveTexture(active[0]);
}

static void barrel_distortion_before(void* distortion)
{
	GLint viewport[4];
	barrel_distortion_t* barrel;
	barrel = (barrel_distortion_t*)distortion;

	memset(viewport, 0, sizeof(viewport));
	glGetIntegerv(GL_VIEWPORT, viewport);
	if (viewport[2] != barrel->fbo.width || viewport[3] != barrel->fbo.height)
	{
		if (viewport[2] > 0 && viewport[3] > 0)
		{
			opengl_fbo_destroy(&barrel->fbo);
			opengl_fbo_create(&barrel->fbo, viewport[2], viewport[3]);
			printf("[GLES2] %s: opengl_fbo_create(%d, %d)\n", __FUNCTION__, viewport[2], viewport[3]);
		}
	}

	if (0 != barrel->fbo.framebuffer)
	{
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&barrel->framebuffer0);
		glBindFramebuffer(GL_FRAMEBUFFER, barrel->fbo.framebuffer);
	}
}

static const char* s_vertex_shader = STRING(
	attribute vec4 v_position;
	attribute vec2 v_texture;
	varying vec2 f_texture;
	
	vec4 Distort(vec4 p)
	{
		vec2 v = p.xy / p.w;
		float radius = length(v);
		if (radius > 0.0)
		{
			float theta = atan(v.y, v.x);

			radius = pow(radius, 0.5);

			v.x = radius * cos(theta);
			v.y = radius * sin(theta);
			p.xy = v.xy * p.w;
		}
		return p;
	}

	void main()
	{
		gl_Position = v_position;
		f_texture = v_texture;
	}
);

static const char* s_fragment_shader = STRING(
	precision highp float;
	uniform sampler2D tex;
	varying vec2 f_texture;

	void main()
	{
		gl_FragColor = texture2D(tex, f_texture);
	}
);

static double barrel_distortion_factor(GLfloat radius)
{
	// http://www.imatest.com/docs/distortion/#algorithm
	// ru = rd + h1 rd3 + h2 rd5 (5th order)

	double radius2 = radius * radius;
	return 1.0 + BARREL_COEFFICIENT_K0 * radius2 + BARREL_COEFFICIENT_K1 * radius2 * radius2;
}

static double barrel_distortion_distort(double radius)
{
	return radius * barrel_distortion_factor(radius);
}

static GLfloat barrel_distortion_distort_inverse(GLfloat radius)
{
	double r0, r1, r2, dr0, dr1;

	r0 = radius / 0.9;
	r1 = radius * 0.9;
	dr0 = radius - barrel_distortion_distort(radius);

	while ( ABS(r1 - r0) > 0.0001 )
	{		
		dr1 = radius - barrel_distortion_distort(r1);
		r2 = r1 - dr1 * ((r1 - r0) / (dr1 - dr0));
		r0 = r1;
		r1 = r2;
		dr0 = dr1;
	}

	return (GLfloat)r1;
}

static void* barrel_distortion_create()
{
	barrel_distortion_t* barrel;
	barrel = (barrel_distortion_t*)malloc(sizeof(barrel_distortion_t));
	if (barrel)
	{
		memset(barrel, 0, sizeof(barrel_distortion_t));
		if (0 != opengl_shader_create(&barrel->shader, s_vertex_shader, s_fragment_shader))
		{
			free(barrel);
			return 0;
		}

		barrel->pid = glGetAttribLocation(barrel->shader.program, "v_position");
		barrel->tid = glGetAttribLocation(barrel->shader.program, "v_texture");
		barrel->uid = glGetUniformLocation(barrel->shader.program, "tex");

		glGenBuffers(2, barrel->buffer);
		glBindBuffer(GL_ARRAY_BUFFER, barrel->buffer[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertex), s_vertex, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, barrel->buffer[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_index), s_index, GL_STATIC_DRAW);		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	return barrel;
}

static void barrel_distortion_destroy(void* distortion)
{
	barrel_distortion_t* barrel;
	barrel = (barrel_distortion_t*)distortion;

	opengl_fbo_destroy(&barrel->fbo);

	opengl_shader_destroy(&barrel->shader);

	glDeleteBuffers(2, barrel->buffer);

	memset(barrel, 0, sizeof(barrel_distortion_t));
}

GLsizei rectangle_vertex_count(int row, int col); 
GLsizei rectangle_index_count(int row, int col);
void rectangle_vertex(int row, int col, GLfloat vertices[]);
void rectangle_index(int row, int col, GLushort indices[]);

distortion_t* distortion_barrel()
{
	static distortion_t s_barrel = {
		barrel_distortion_create,
		barrel_distortion_destroy,
		barrel_distortion_before,
		barrel_distortion_after,
	};

	static int i = 0;
	if (0 == i)
	{
		i = 1;
		assert(rectangle_vertex_count(ROW, COL) * 4 == sizeof(s_vertex) / sizeof(s_vertex[0]));
		assert(rectangle_index_count(ROW, COL) == sizeof(s_index) / sizeof(s_index[0]));
		rectangle_vertex(ROW, COL, s_vertex);
		rectangle_index(ROW, COL, s_index);

		for (i = 0; i < rectangle_vertex_count(ROW, COL); i++)
		{
			GLfloat radius, x, y;
			x = s_vertex[i * 4 + 0];
			y = s_vertex[i * 4 + 1];
			radius = sqrt(x * x + y * y);
			radius = radius > 0.0 ? barrel_distortion_distort_inverse(radius) / radius : 1.0;

			s_vertex[i * 4 + 0] *= radius;
			s_vertex[i * 4 + 1] *= radius;
		}
	}

	return &s_barrel;
}
