#ifndef _environment_map_h_
#define _environment_map_h_

#include "opengl-es2.h"

#if defined(__cplusplus) || defined(_cplusplus)
extern "C"
{
#endif

	typedef struct _environment_map_t
	{
		void* (*create)();
		void (*destroy)(void* proj);
		void (*draw)(void* proj, GLuint v4Position, GLuint uViewMatrix, const GLfloat viewMatrix[16], GLuint v2Texture, GLuint uTextureMatrix, const GLfloat textureMatrix[16]);
	} environment_map_t;

	environment_map_t* environment_map_plane();
	environment_map_t* environment_map_spherical();
	environment_map_t* environment_map_equirectangular();

#if defined(__cplusplus) || defined(_cplusplus)
}
#endif
#endif
