#include "opengl-es2.h"
#include <assert.h>
#include <math.h>

#define PI			3.14159265358979323846
#define PI_2		1.57079632679489661923   // pi / 2
#define PI2			6.28318530717958647692   // pi * 2

GLsizei sphere_vertex_count(int stacks, int slices)
{
	return ((stacks + 1) * (slices + 1));
}

GLsizei sphere_index_count(int stacks, int slices)
{
	return (stacks - 1) * slices * 6;
}

void sphere_vertex(int stacks, int slices, GLfloat* positions)
{
	int i;
	int stack;
	int slice;
	double x, y, z, rxz;
	double phi, theta;

	i = 0;

	// right hander coordination
	for (stack = 0; stack <= stacks; stack++)
	{
		phi = PI_2 - stack * PI / stacks;
		y = sin(phi);
		rxz = cos(phi);
		assert(rxz >= 0);

		for (slice = 0; slice <= slices; slice++)
		{
			theta = PI2 * slice / slices;
			x = rxz * cos(theta);
			z = rxz * sin(theta);

			positions[i++] = (GLfloat)x;
			positions[i++] = (GLfloat)y;
			positions[i++] = (GLfloat)z;
			positions[i++] = 1.0f * slice / slices; // u-texture coordinate
			positions[i++] = 1.0f * stack / stacks; // v-texture coordinate
		}
	}

	assert(i == sphere_vertex_count(stacks, slices) * 5);
}

void sphere_index(int stacks, int slices, GLuint* indices)
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
				indices[i++] = (GLuint)(bot + slice);
				indices[i++] = (GLuint)(top + slice);
				indices[i++] = (GLuint)(top + slice + 1);
			}

			if (stack != stacks - 1)
			{
				indices[i++] = (GLuint)(bot + slice);
				indices[i++] = (GLuint)(top + slice + 1);
				indices[i++] = (GLuint)(bot + slice + 1);
			}
		}
	}

	assert(i == sphere_index_count(stacks, slices));
}
