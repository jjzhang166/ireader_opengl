#include "internal.h"
#include "distortion.h"
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <string.h>

#define STRINGZ(x)	#x
#define STRINGZ2(x)	STRINGZ(x)
#define STRING(x)	STRINGZ2(x)

#define ROW 40
#define COL 40

static GLushort s_index[ROW * COL * 6];
static GLfloat s_vertex[(ROW + 1) * (COL + 1) * 4];

//static GLushort s_index[] = {
//	0, 1, 2,
//	2, 1, 3,
//};
//static GLfloat s_vertex[] = {
//	-1.0f, 1.0f, 0.0f, 1.0f,
//	-1.0f, -1.0f, 0.0f, 0.0f,
//	1.0f, 1.0f, 1.0f, 1.0f,
//	1.0f, -1.0f, 1.0f, 0.0f,
//};

typedef struct _barrel_distortion_t
{
	GLuint framebuffer0;
	GLuint framebuffer;
	GLuint renderbuffer;
	GLuint texture;
	GLuint program;
	GLuint buffer[2];
	GLint viewport[4];

	GLint pid; // attribute v_position
	GLint tid; // attribute v_texture
	GLint uid; // uniform sampler2D tex0
} barrel_distortion_t;

static void barrel_distortion_render(barrel_distortion_t* barrel)
{
	glBindBuffer(GL_ARRAY_BUFFER, barrel->buffer[0]);
	glVertexAttribPointer(barrel->pid, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (const void*)0);
	glEnableVertexAttribArray(barrel->pid);
	glVertexAttribPointer(barrel->tid, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (const void*)(2 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(barrel->tid);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, barrel->texture);
	glUniform1i(barrel->uid, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, barrel->buffer[1]);
	glDrawElements(GL_TRIANGLES, sizeof(s_index) / sizeof(s_index[0]), GL_UNSIGNED_SHORT, 0);
}

static void barrel_distortion_setup(void* distortion, int width, int height)
{
	GLenum errcode;
	GLint framebuffer[1];
	barrel_distortion_t* barrel;
	barrel = (barrel_distortion_t*)distortion;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, framebuffer);

	glGenTextures(1, &barrel->texture);
	glBindTexture(GL_TEXTURE_2D, barrel->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	//glGenRenderbuffers(1, &barrel->renderbuffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, barrel->renderbuffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &barrel->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, barrel->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, barrel->texture, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, barrel->renderbuffer);

	errcode = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != errcode)
	{
		printf("[GLES2] framebuffer is not complete: %d\n", (int)errcode);
		ALOGE("[GLES2] framebuffer is not complete: %d\n", (int)errcode);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[0]); // restore framebuffer
}

static void barrel_distortion_before(void* distortion)
{
	GLint viewport[4];
	barrel_distortion_t* barrel;
	barrel = (barrel_distortion_t*)distortion;

	glGetIntegerv(GL_VIEWPORT, viewport);
	if (viewport[0] != barrel->viewport[0] || viewport[1] != barrel->viewport[1]
		|| viewport[2] != barrel->viewport[2] || viewport[3] != barrel->viewport[3])
	{
		if (barrel->renderbuffer)
		{
			glDeleteRenderbuffers(1, &barrel->renderbuffer);
			barrel->renderbuffer = 0;
		}

		if (barrel->framebuffer)
		{
			glDeleteFramebuffers(1, &barrel->framebuffer);
			barrel->framebuffer = 0;
		}

		if (barrel->texture)
		{
			glDeleteTextures(1, &barrel->texture);
			barrel->texture = 0;
		}
	}

	if (0 == barrel->framebuffer)
	{
		ALOGI("[GLES2] barrel_distortion_setup view: %d/%d/%d/%d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
		barrel_distortion_setup(barrel, viewport[2], viewport[3]);
		barrel->viewport[0] = viewport[0];
		barrel->viewport[1] = viewport[1];
		barrel->viewport[2] = viewport[2];
		barrel->viewport[3] = viewport[3];
	}

	if (barrel->framebuffer)
	{
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&barrel->framebuffer0);
		glBindFramebuffer(GL_FRAMEBUFFER, barrel->framebuffer);
	}
}

static void barrel_distortion_after(void* distortion)
{
	GLint program[1];
	GLint viewport[4];
	barrel_distortion_t* barrel;
	barrel = (barrel_distortion_t*)distortion;
	assert(barrel->program);

	if (0 == barrel->framebuffer)
		return;

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetIntegerv(GL_CURRENT_PROGRAM, program);

	glBindFramebuffer(GL_FRAMEBUFFER, barrel->framebuffer0);
	glUseProgram(barrel->program);

	glViewport(barrel->viewport[0], barrel->viewport[1], barrel->viewport[2], barrel->viewport[3]);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	//glEnable(GL_SCISSOR_TEST);
	glViewport(barrel->viewport[0], barrel->viewport[1], barrel->viewport[2] / 2, barrel->viewport[3]);
	barrel_distortion_render(barrel);

	glViewport(barrel->viewport[0] + barrel->viewport[2] / 2, barrel->viewport[1], barrel->viewport[2] / 2, barrel->viewport[3]);
	barrel_distortion_render(barrel);

	glDisableVertexAttribArray(barrel->pid);
	glDisableVertexAttribArray(barrel->tid);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(program[0]);
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
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
		gl_Position = Distort(v_position);
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
		ALOGE("[GLES2] load shader failed: %s\n%s", msg, string);
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
		ALOGE("[GLES2] load program failed: %s\n", msg);
		glDeleteProgram(program);
		program = 0;
	}
	return program;
}

static GLuint barrel_distortion_load_program()
{
	GLuint vertex;
	GLuint fragment;
	GLuint program;

	vertex = opengl_load_shader(GL_VERTEX_SHADER, s_vertex_shader);
	fragment = opengl_load_shader(GL_FRAGMENT_SHADER, s_fragment_shader);
	if (0 == vertex || 0 == fragment)
		return 0;

	program = opengl_load_program(vertex, fragment);

	glDeleteShader(vertex);		// flagged for deletion
	glDeleteShader(fragment);	// flagged for deletion
	return program;
}

static int barrel_distortion_vertex(GLfloat vertices[])
{
	int i, r, c;
	GLfloat u, v;

	i = 0;
	for (r = 0; r <= ROW; r++)
	{
		for (c = 0; c <= COL; c++)
		{
			u = 1.0f * r / ROW;
			v = 1.0f * c / COL;

			vertices[i++] = 2 * u - 1.0f; // x-coordinate
			vertices[i++] = 2 * v - 1.0f; // y-coordinate

			vertices[i++] = u / 2; // texture-u
			vertices[i++] = v / 2; // texture-v
		}
	}

	return i;
}

static int barrel_distortion_index(GLushort indices[])
{
	int i, r, c, top, bot;

	i = 0;
	for (r = 0; r < ROW; r++)
	{
		for (c = 0; c < COL; c++)
		{
			top = r * (COL + 1);
			bot = (r + 1) * (COL + 1);

			indices[i++] = top + c;
			indices[i++] = bot + c;
			indices[i++] = top + c + 1;

			indices[i++] = top + c + 1;
			indices[i++] = bot + c;
			indices[i++] = bot + c + 1;
		}
	}

	return i;
}

static void* barrel_distortion_create()
{
	barrel_distortion_t* barrel;
	barrel = (barrel_distortion_t*)malloc(sizeof(barrel_distortion_t));
	if (barrel)
	{
		memset(barrel, 0, sizeof(barrel_distortion_t));
		barrel->program = barrel_distortion_load_program();
		if (0 == barrel->program)
		{
			free(barrel);
			return 0;
		}

		barrel->pid = glGetAttribLocation(barrel->program, "v_position");
		barrel->tid = glGetAttribLocation(barrel->program, "v_texture");
		barrel->uid = glGetUniformLocation(barrel->program, "tex");

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

	if (barrel->renderbuffer)
	{
		glDeleteRenderbuffers(1, &barrel->renderbuffer);
	}

	if (barrel->framebuffer)
	{
		glDeleteFramebuffers(1, &barrel->framebuffer);
	}

	if (barrel->texture)
	{
		glDeleteTextures(1, &barrel->texture);
	}

	glDeleteProgram(barrel->program); // A value of 0 will be silently ignored

	glDeleteBuffers(2, barrel->buffer);

	memset(barrel, 0, sizeof(barrel_distortion_t));
}

distortion_t* distortion_barrel()
{
	static distortion_t s_barrel = {
		barrel_distortion_create,
		barrel_distortion_destroy,
//		barrel_distortion_setup,
		barrel_distortion_before,
		barrel_distortion_after,
	};

	static int i = 0;
	if (0 == i)
	{
		barrel_distortion_index(s_index);
		barrel_distortion_vertex(s_vertex);
	}

	return &s_barrel;
}
