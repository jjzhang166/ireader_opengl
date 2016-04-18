#ifndef _distortion_h_
#define _distortion_h_

#include "opengl-es2.h"

typedef struct _distortion_t
{
	void* (*create)();
	void (*destroy)(void* distortion);

	void (*before_draw)(void* distortion);
	void (*after_draw)(void* distortion);
} distortion_t;

distortion_t* distortion_barrel();
distortion_t* distortion_pincushion();
distortion_t* distortion_mustache();
distortion_t* distortion_spherical();

#endif /* !_distortion_h_ */
