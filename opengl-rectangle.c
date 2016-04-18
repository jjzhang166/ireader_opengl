#include "opengl-es2.h"
#include <assert.h>

GLsizei rectangle_vertex_count(int row, int col)
{
	return (row + 1) * (col + 1);
}

GLsizei rectangle_index_count(int row, int col)
{
	return row * col * 6;
}

void rectangle_vertex(int row, int col, GLfloat vertices[])
{
	int i, r, c;
	GLfloat u, v;

	i = 0;
	for (r = 0; r <= row; r++)
	{
		for (c = 0; c <= col; c++)
		{
			u = 1.0f * r / row;
			v = 1.0f * c / col;

			vertices[i++] = 2 * u - 1.0f; // x-coordinate
			vertices[i++] = 2 * v - 1.0f; // y-coordinate

			vertices[i++] = u / 2; // texture-u
			vertices[i++] = v / 2; // texture-v
		}
	}

	assert(i == rectangle_vertex_count(row, col) * 4);
}

void rectangle_index(int row, int col, GLushort indices[])
{
	int i, r, c, top, bot;

	i = 0;
	for (r = 0; r < row; r++)
	{
		for (c = 0; c < col; c++)
		{
			top = r * (col + 1);
			bot = (r + 1) * (col + 1);

			indices[i++] = top + c;
			indices[i++] = bot + c;
			indices[i++] = top + c + 1;

			indices[i++] = top + c + 1;
			indices[i++] = bot + c;
			indices[i++] = bot + c + 1;
		}
	}

	assert(i == rectangle_index_count(row, col));
}
