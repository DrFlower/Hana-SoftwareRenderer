#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "camera.h"
#include "macro.h"
#include "maths.h"
#include "iostream"

/*
 * for orbital camera controls, see
 * https://github.com/mrdoob/three.js/blob/master/examples/js/controls/OrbitControls.js
 */

static const float NEAR = 0.1f;
static const float FAR = 10000;
static const float FOVY = TO_RADIANS(60);
static const Vector3f UP = { 0, 1, 0 };

struct camera {
	Vector3f position;
	Vector3f target;
	float aspect;
};

/* camera creating/releasing */

camera_t* camera_create(Vector3f position, Vector3f target, float aspect) {
	camera_t* camera;

	assert((position - target).normal() > EPSILON && aspect > 0);

	camera = (camera_t*)malloc(sizeof(camera_t));
	camera->position = position;
	camera->target = target;
	camera->aspect = aspect;

	return camera;
}

void camera_release(camera_t* camera) {
	free(camera);
}

/* camera updating */

void camera_set_transform(camera_t* camera, Vector3f position, Vector3f target) {
	assert((position - target).normal() > EPSILON);
	camera->position = position;
	camera->target = target;
}

static Vector3f calculate_pan(Vector3f from_camera, motion_t motion) {
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

static Vector3f calculate_offset(Vector3f from_target, motion_t motion) {
	float radius = from_target.normal();
	float theta = (float)atan2(from_target.x, from_target.z);  /* azimuth */
	float phi = (float)acos(from_target.y / radius);           /* polar */
	float factor = PI * 2;
	Vector3f offset;

	radius *= (float)pow(0.95, motion.dolly);
	theta -= motion.orbit.x * factor;
	phi -= motion.orbit.y * factor;
	phi = float_clamp(phi, EPSILON, PI - EPSILON);

	offset.x = radius * (float)sin(phi) * (float)sin(theta);
	offset.y = radius * (float)cos(phi);
	offset.z = radius * (float)sin(phi) * (float)cos(theta);

	return offset;
}

void camera_update_transform(camera_t* camera, motion_t motion) {
	Vector3f from_target = camera->position - camera->target;
	Vector3f from_camera = camera->target - camera->position;
	Vector3f pan = calculate_pan(from_camera, motion);
	Vector3f offset = calculate_offset(from_target, motion);
	camera->target = camera->target + pan;
	camera->position = camera->target + offset;
	//std::cout << "camera pos:" << camera->position.x << "," << camera->position.y << "," << camera->position.z << "," << std::endl;
}

/* property retrieving */

Vector3f camera_get_position(camera_t* camera) {
	return camera->position;
}

Vector3f camera_get_forward(camera_t* camera) {
	return (camera->target - camera->position).normalize();
}

Matrix4x4 camera_get_view_matrix(camera_t* camera) {
	vec3_t position = vec3_new(camera->position.x, camera->position.y, camera->position.z);
	vec3_t target = vec3_new(camera->target.x, camera->target.y, camera->target.z);
	vec3_t up = vec3_new(UP.x, UP.y, UP.z);
	mat4_t m = mat4_lookat(position, target, up);
	Matrix4x4 ret = Matrix4x4::identity();
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			ret[i][j] = m.m[i][j];
		}
	}
	return ret;
}

Matrix4x4 camera_get_proj_matrix(camera_t* camera) {
	mat4_t m = mat4_perspective(FOVY, camera->aspect, NEAR, FAR);
	Matrix4x4 ret = Matrix4x4::identity();
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			ret[i][j] = m.m[i][j];
		}
	}
	return ret;
}
