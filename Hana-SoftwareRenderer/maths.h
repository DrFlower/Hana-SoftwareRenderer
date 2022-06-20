#pragma once

#include "vector.h"
#include "matrix.h"

float clamp(float f, float min, float max);
float saturate(float f);

Matrix4x4 lookat(Vector3f eye, Vector3f target, Vector3f up);
Matrix4x4 orthographic(float right, float top, float near, float far);
Matrix4x4 perspective(float fovy, float aspect, float near, float far);