#include <assert.h>
#include <stdlib.h>
#include "camera.h"
#include "macro.h"
#include "iostream"

/*
 * for orbital camera controls, see
 * https://github.com/mrdoob/three.js/blob/master/examples/js/controls/OrbitControls.js
 */

static const float NEAR = 0.1f;
static const float FAR = 10000;
static const float FOVY = TO_RADIANS(60);
static const Vector3f UP = { 0, 1, 0 };

Camera::Camera(Vector3f position, Vector3f target, float aspect) {
	assert((position - target).normal() > EPSILON && aspect > 0);
	this->position = position;
	this->target = target;
	this->aspect = aspect;
}

/* camera updating */

void Camera::set_transform(Vector3f position, Vector3f target) {
	assert((position - target).normal() > EPSILON);
	this->position = position;
	this->target = target;
}

static Vector3f calculate_pan(Vector3f from_camera, Motion motion) {
	Vector3f _from_camera = from_camera;
	Vector3f forward = _from_camera.normalize();
	Vector3f left = cross(UP, forward);
	Vector3f up = cross(forward, left);

	float distance = from_camera.normal();
	float factor = distance * (float)tan(FOVY / 2) * 2;
	Vector3f delta_x = left * motion.pan.x * factor;
	Vector3f delta_y = up * motion.pan.y * factor;
	return delta_x + delta_y;
}

static Vector3f calculate_offset(Vector3f from_target, Motion motion) {
	float radius = from_target.normal();
	float theta = (float)atan2(from_target.x, from_target.z);  /* azimuth */
	float phi = (float)acos(from_target.y / radius);           /* polar */
	float factor = PI * 2;
	Vector3f offset;

	radius *= (float)pow(0.95, motion.dolly);
	theta -= motion.orbit.x * factor;
	phi -= motion.orbit.y * factor;
	phi = clamp(phi, EPSILON, PI - EPSILON);

	offset.x = radius * (float)sin(phi) * (float)sin(theta);
	offset.y = radius * (float)cos(phi);
	offset.z = radius * (float)sin(phi) * (float)cos(theta);

	return offset;
}

void Camera::update_transform(Motion motion) {
	Vector3f from_target = position - target;
	Vector3f from_camera = target - position;
	Vector3f pan = calculate_pan(from_camera, motion);
	Vector3f offset = calculate_offset(from_target, motion);
	target = target + pan;
	position = target + offset;
}

/* property retrieving */

Vector3f Camera::get_position() {
	return position;
}

Vector3f Camera::get_forward() {
	return (target - position).normalize();
}

Matrix4x4 Camera::get_view_matrix() {
	Vector3f up = Vector3f(UP.x, UP.y, UP.z);
	Matrix4x4 m = lookat(position, target, up);
	Matrix4x4 ret = Matrix4x4::identity();
	return m;
}

Matrix4x4 Camera::get_proj_matrix() {
	Matrix4x4 m = perspective(FOVY, aspect, NEAR, FAR);
	return m;
}

Vector3f Camera::get_target_position() {
	return target;
}
