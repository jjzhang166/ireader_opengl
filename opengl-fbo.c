#include "opengl-fbo.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

int opengl_fbo_create(struct opengl_fbo_t* fbo, int width, int height)
{
	GLenum errcode;
	GLuint framebuffer[1];

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)framebuffer);

	glGenTextures(1, &fbo->texture);
	glBindTexture(GL_TEXTURE_2D, fbo->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &fbo->renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, fbo->renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &fbo->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->renderbuffer);

	errcode = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != errcode)
	{
		printf("[GLES2] framebuffer is not complete: %d\n", (int)errcode);
	}

	fbo->width = width;
	fbo->height = height;

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[0]); // restore framebuffer
	return (int)errcode;
}

int opengl_fbo_destroy(struct opengl_fbo_t* fbo)
{
	if (!fbo)
		return 0;

	if (fbo->framebuffer)
	{
		glDeleteFramebuffers(1, &fbo->framebuffer);
	}

	if (fbo->renderbuffer)
	{
		glDeleteRenderbuffers(1, &fbo->renderbuffer);
	}

	if (fbo->texture)
	{
		glDeleteTextures(1, &fbo->texture);
	}

	memset(fbo, 0, sizeof(struct opengl_fbo_t));
	return 0;
}
