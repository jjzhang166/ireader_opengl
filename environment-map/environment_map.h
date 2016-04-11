#ifndef _environment_map_h_
#define _environment_map_h_

#include "ijksdl/ijksdl_gles2.h"

#if defined(__cplusplus) || defined(_cplusplus)
extern "C"
{
#endif

	typedef struct _environment_map_t
	{
		void* (*create)();
		void (*destroy)(void* proj);
		void (*setup)(void* proj, GLuint v4Position, GLuint v2Texture);
		void (*draw)(void* proj, GLsizei width, GLsizei height, GLuint mat4MVP, const GLfloat viewMatrix[16]);
	} environment_map_t;

	environment_map_t* environment_map_plane();
	environment_map_t* environment_map_shperical();
	environment_map_t* environment_map_equirectangular();

#if defined(__cplusplus) || defined(_cplusplus)
}
#endif
#endif
