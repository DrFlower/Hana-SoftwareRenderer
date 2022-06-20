#include "maths.h"

float clamp(float f, float min, float max) {
	return f < min ? min : (f > max ? max : f);
}

float saturate(float f) {
    return f < 0 ? 0 : (f > 1 ? 1 : f);
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
 *     mat4_ortho(left, right, bottom, top, near, far);
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
 *     mat4_frustum(-half_w, half_w, -half_h, half_h, near, far);
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