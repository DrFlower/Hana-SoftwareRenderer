#pragma once

#include "vector.h"
#include "matrix.h"

float clamp(float f, float min, float max);
float saturate(float f);

/* transformation matrices */
Matrix4x4 translate(float tx, float ty, float tz);
Matrix4x4 scale(float sx, float sy, float sz);
Matrix4x4 rotate(float angle, float vx, float vy, float vz);
Matrix4x4 rotate_x(float angle);
Matrix4x4 rotate_y(float angle);
Matrix4x4 rotate_z(float angle);
Matrix4x4 lookat(Vector3f eye, Vector3f target, Vector3f up);
Matrix4x4 orthographic(float right, float top, float near, float far);
Matrix4x4 perspective(float fovy, float aspect, float near, float far);