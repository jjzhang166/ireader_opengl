#include "opengl-es2.h"
#include <math.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

#define PI	3.14159265358979323846

void opengl_matrix_identity(GLfloat m[16])
{
	memset(m, 0, sizeof(GLfloat) * 16);
	m[0] = 1.0f;
	m[5] = 1.0f;
	m[10] = 1.0f;
	m[15] = 1.0f;
}

void opengl_matrix_ortho(GLfloat m[16], GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far)
{
	m[0] = 2.0f / (right - left);
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = 2.0f / (top - bottom);
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = 2.0f / (near - far);
	m[11] = 0.0f;

	m[12] = (left + right) / (left - right);
	m[13] = (bottom + top) / (bottom - top);
	m[14] = (far + near) / (far - near);
	m[15] = 1.0f;
}

void opengl_matrix_frustum(GLfloat m[16], GLfloat left, GLfloat right, GLfloat top, GLfloat bottom, GLfloat near, GLfloat far)
{
	m[0] = 2 * near / (right - left);
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = 2 * near / (top - bottom);
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = (right + left) / (right - left);
	m[9] = (top + bottom) / (top - bottom);
	m[10] = (near + far) / (near - far);
	m[11] = -1.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 2 * near * far / (near - far);
	m[15] = 0.0f;
}

void opengl_matrix_perspective(GLfloat m[16], GLfloat fovy, GLfloat aspect, GLfloat near, GLfloat far)
{
	GLfloat f = 1.0f / tan(fovy * PI / 360.0);

	m[0] = f / aspect;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = f;
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = (near + far) / (near - far);
	m[11] = -1.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 2.0f * far * near / (near - far);
	m[15] = 0.0f;
}

#define I(_i, _j) ((_j)+ 4*(_i))
void opengl_matrix_multiply_mm(GLfloat m[16], const GLfloat lhs[16], const GLfloat rhs[16])
{
	for (int i = 0; i < 4; i++)
	{
		register const float rhs_i0 = rhs[I(i, 0)];
		register float ri0 = lhs[I(0, 0)] * rhs_i0;
		register float ri1 = lhs[I(0, 1)] * rhs_i0;
		register float ri2 = lhs[I(0, 2)] * rhs_i0;
		register float ri3 = lhs[I(0, 3)] * rhs_i0;

		for (int j = 1; j < 4; j++)
		{
			register const float rhs_ij = rhs[I(i, j)];
			ri0 += lhs[I(j, 0)] * rhs_ij;
			ri1 += lhs[I(j, 1)] * rhs_ij;
			ri2 += lhs[I(j, 2)] * rhs_ij;
			ri3 += lhs[I(j, 3)] * rhs_ij;
		}
		m[I(i, 0)] = ri0;
		m[I(i, 1)] = ri1;
		m[I(i, 2)] = ri2;
		m[I(i, 3)] = ri3;
	}
}

/*
Rotate X
|    1        0        0        0    |
|                                    |
|    0      cos(¦È)   sin(¦È)     0    |
|                                    |
|    0     -sin(¦È)   cos(¦È)     0    |
|                                    |
|    0        0        0        1    |

Rotate Y
|  cos(¦È)     0    -sin(¦È)      0    |
|                                    |
|    0        1        0        0    |
|                                    |
|  sin(¦È)     0     cos(¦È)      0    |
|                                    |
|    0        0        0        1    |

Rotate Z
|  cos(¦È)  -sin(¦È)     0        0    |
|                                    |
|  sin(¦È)   cos(¦È)     0        0    |
|                                    |
|    0        0        1        0    |
|                                    |
|    0        0        0        1    |
*/
void opengl_matrix_rotate_x(GLfloat m[16], GLfloat angle)
{
	memset(m, 0, sizeof(GLfloat) * 16);
	m[0] = 1.0f;
	m[5] = cos(angle);
	m[6] = -sin(angle);
	m[9] = sin(angle);
	m[10] = cos(angle);
	m[15] = 1.0f;
}

void opengl_matrix_rotate_y(GLfloat m[16], GLfloat angle)
{
	memset(m, 0, sizeof(GLfloat) * 16);
	m[0] = cos(angle);
	m[2] = -sin(angle);
	m[5] = 1.0f;
	m[8] = sin(angle);
	m[10] = cos(angle);
	m[15] = 1.0f;
}

void opengl_matrix_translate(GLfloat m[16], GLfloat x, GLfloat y, GLfloat z)
{
	memset(m, 0, sizeof(GLfloat) * 16);
	m[0] = 1.0f;
	m[5] = 1.0f;
	m[10] = 1.0f;

	m[12] = x;
	m[13] = y;
	m[14] = z;
	m[15] = 1.0f;
}

void opengl_matrix_scale(GLfloat m[16], GLfloat x, GLfloat y, GLfloat z)
{
	memset(m, 0, sizeof(GLfloat) * 16);
	m[0] = x;
	m[5] = y;
	m[10] = z;
	m[15] = 1.0f;
}
