#include "maths.h"

float clamp(float f, float min, float max) {
	return f < min ? min : (f > max ? max : f);
}

float saturate(float f) {
	return f < 0 ? 0 : (f > 1 ? 1 : f);
}

float lerp(float a, float b, float t) {
	return a + (b - a) * t;
}


/*
 * for viewport transformation, see subsection 2.12.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
Vector3f viewport_transform(int width, int height, Vector3f ndc_coord) {
	float x = (ndc_coord.x + 1) * 0.5f * (float)width;   /* [-1, 1] -> [0, w] */
	float y = (ndc_coord.y + 1) * 0.5f * (float)height;  /* [-1, 1] -> [0, h] */
	float z = (ndc_coord.z + 1) * 0.5f;                  /* [-1, 1] -> [0, 1] */
	return Vector3f(x, y, z);
}

/* transformation matrices */

/*
 * tx, ty, tz: the x, y, and z coordinates of a translation vector
 *
 *  1  0  0 tx
 *  0  1  0 ty
 *  0  0  1 tz
 *  0  0  0  1
 *
 * see http://docs.gl/gl2/glTranslate
 */
Matrix4x4 translate(float tx, float ty, float tz) {
	Matrix4x4 m = Matrix4x4::identity();
	m[0][3] = tx;
	m[1][3] = ty;
	m[2][3] = tz;
	return m;
}

/*
 * sx, sy, sz: scale factors along the x, y, and z axes, respectively
 *
 * sx  0  0  0
 *  0 sy  0  0
 *  0  0 sz  0
 *  0  0  0  1
 *
 * see http://docs.gl/gl2/glScale
 */
Matrix4x4 scale(float sx, float sy, float sz) {
	Matrix4x4 m = Matrix4x4::identity();
	assert(sx != 0 && sy != 0 && sz != 0);
	m[0][0] = sx;
	m[1][1] = sy;
	m[2][2] = sz;
	return m;
}

/*
 * angle: the angle of rotation, in radians
 * vx, vy, vz: the x, y, and z coordinates of a vector, respectively
 *
 * nx*nx*(1-c)+c     ny*nx*(1-c)-s*nz  nz*nx*(1-c)+s*ny  0
 * nx*ny*(1-c)+s*nz  ny*ny*(1-c)+c     nz*ny*(1-c)-s*nx  0
 * nx*nz*(1-c)-s*ny  ny*nz*(1-c)+s*nx  nz*nz*(1-c)+c     0
 * 0                 0                 0                 1
 *
 * nx, ny, nz: the normalized coordinates of the vector, respectively
 * s, c: sin(angle), cos(angle)
 *
 * see http://docs.gl/gl2/glRotate
 */
Matrix4x4 rotate(float angle, float vx, float vy, float vz) {
	Vector3f n = Vector3f(vx, vy, vz).normalize();
	float c = (float)cos(angle);
	float s = (float)sin(angle);
	Matrix4x4 m = Matrix4x4::identity();

	m[0][0] = n.x * n.x * (1 - c) + c;
	m[0][1] = n.y * n.x * (1 - c) - s * n.z;
	m[0][2] = n.z * n.x * (1 - c) + s * n.y;

	m[1][0] = n.x * n.y * (1 - c) + s * n.z;
	m[1][1] = n.y * n.y * (1 - c) + c;
	m[1][2] = n.z * n.y * (1 - c) - s * n.x;

	m[2][0] = n.x * n.z * (1 - c) - s * n.y;
	m[2][1] = n.y * n.z * (1 - c) + s * n.x;
	m[2][2] = n.z * n.z * (1 - c) + c;

	return m;
}

/*
 * angle: the angle of rotation, in radians
 *
 *  1  0  0  0
 *  0  c -s  0
 *  0  s  c  0
 *  0  0  0  1
 *
 * see http://www.songho.ca/opengl/gl_anglestoaxes.html
 */
Matrix4x4 rotate_x(float angle) {
	float c = (float)cos(angle);
	float s = (float)sin(angle);
	Matrix4x4 m = Matrix4x4::identity();
	m[1][1] = c;
	m[1][2] = -s;
	m[2][1] = s;
	m[2][2] = c;
	return m;
}

/*
 * angle: the angle of rotation, in radians
 *
 *  c  0  s  0
 *  0  1  0  0
 * -s  0  c  0
 *  0  0  0  1
 *
 * see http://www.songho.ca/opengl/gl_anglestoaxes.html
 */
Matrix4x4 rotate_y(float angle) {
	float c = (float)cos(angle);
	float s = (float)sin(angle);
	Matrix4x4 m = Matrix4x4::identity();
	m[0][0] = c;
	m[0][2] = s;
	m[2][0] = -s;
	m[2][2] = c;
	return m;
}

/*
 * angle: the angle of rotation, in radians
 *
 *  c -s  0  0
 *  s  c  0  0
 *  0  0  1  0
 *  0  0  0  1
 *
 * see http://www.songho.ca/opengl/gl_anglestoaxes.html
 */
Matrix4x4 rotate_z(float angle) {
	float c = (float)cos(angle);
	float s = (float)sin(angle);
	Matrix4x4 m = Matrix4x4::identity();
	m[0][0] = c;
	m[0][1] = -s;
	m[1][0] = s;
	m[1][1] = c;
	return m;
}

/*
 * eye: the position of the eye point
 * target: the position of the target point
 * up: the direction of the up vector
 *
 * x_axis.x  x_axis.y  x_axis.z  -dot(x_axis,eye)
 * y_axis.x  y_axis.y  y_axis.z  -dot(y_axis,eye)
 * z_axis.x  z_axis.y  z_axis.z  -dot(z_axis,eye)
 *        0         0         0                 1
 *
 * z_axis: normalize(eye-target), the backward vector
 * x_axis: normalize(cross(up,z_axis)), the right vector
 * y_axis: cross(z_axis,x_axis), the up vector
 *
 * see http://www.songho.ca/opengl/gl_camera.html
 */
Matrix4x4 lookat(Vector3f eye, Vector3f target, Vector3f up) {
	Vector3f z_axis = (eye - target).normalize();
	Vector3f x_axis = cross(up, z_axis).normalize();
	Vector3f y_axis = cross(z_axis, x_axis);
	Matrix4x4 m = Matrix4x4::identity();

	m[0] = embed<4>(x_axis, 0.f);
	m[1] = embed<4>(y_axis, 0.f);
	m[2] = embed<4>(z_axis, 0.f);

	m[0][3] = -(x_axis * eye);
	m[1][3] = -(y_axis * eye);
	m[2][3] = -(z_axis * eye);

	return m;
}

/*
 * right: the coordinates for the right clipping planes (left == -right)
 * top: the coordinates for the top clipping planes (bottom == -top)
 * near, far: the distances to the near and far depth clipping planes
 *
 * 1/r    0         0             0
 *   0  1/t         0             0
 *   0    0  -2/(f-n)  -(f+n)/(f-n)
 *   0    0         0             1
 *
 * this is the same as
 *     float left = -right;
 *     float bottom = -top;
 *     ortho(left, right, bottom, top, near, far);
 *
 * see http://www.songho.ca/opengl/gl_projectionmatrix.html
 */
Matrix4x4 orthographic(float right, float top, float near, float far) {
	float z_range = far - near;
	Matrix4x4 m = Matrix4x4::identity();
	assert(right > 0 && top > 0 && z_range > 0);
	m[0][0] = 1 / right;
	m[1][1] = 1 / top;
	m[2][2] = -2 / z_range;
	m[2][3] = -(near + far) / z_range;
	return m;
}

/*
 * fovy: the field of view angle in the y direction, in radians
 * aspect: the aspect ratio, defined as width divided by height
 * near, far: the distances to the near and far depth clipping planes
 *
 * 1/(aspect*tan(fovy/2))              0             0           0
 *                      0  1/tan(fovy/2)             0           0
 *                      0              0  -(f+n)/(f-n)  -2fn/(f-n)
 *                      0              0            -1           0
 *
 * this is the same as
 *     float half_h = near * (float)tan(fovy / 2);
 *     float half_w = half_h * aspect;
 *     frustum(-half_w, half_w, -half_h, half_h, near, far);
 *
 * see http://www.songho.ca/opengl/gl_projectionmatrix.html
 */

Matrix4x4 perspective(float fovy, float aspect, float near, float far) {
	float z_range = far - near;
	Matrix4x4 m = Matrix4x4::identity();
	assert(fovy > 0 && aspect > 0);
	assert(near > 0 && far > 0 && z_range > 0);
	m[1][1] = 1 / (float)tan(fovy / 2);
	m[0][0] = m[1][1] / aspect;
	m[2][2] = -(near + far) / z_range;
	m[2][3] = -2 * near * far / z_range;
	m[3][2] = -1;
	m[3][3] = 0;
	return m;
}