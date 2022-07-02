#include <assert.h>
#include <stdlib.h>
#include "camera.h"
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
	//return target;
	return Vector3f::Zero;
}

Vector2f get_pos_delta(Vector2f old_pos, Vector2f new_pos, float window_height) {
	Vector2f delta = new_pos - old_pos;
	return delta / window_height;
}

Vector2f get_cursor_pos(window_t* window) {
	float xpos, ypos;
	input_query_cursor(window, &xpos, &ypos);
	return Vector2f(xpos, ypos);
}

void scroll_callback(window_t* window, float offset) {
	Record* record = (Record*)window_get_userdata(window);
	record->dolly_delta += offset;
}

void button_callback(window_t* window, button_t button, int pressed) {
	Record* record = (Record*)window_get_userdata(window);
	Vector2f cursor_pos = get_cursor_pos(window);
	if (button == BUTTON_L) {
		float curr_time = platform_get_time();
		if (pressed) {
			record->is_orbiting = 1;
			record->orbit_pos = cursor_pos;
			record->press_time = curr_time;
			record->press_pos = cursor_pos;
		}
		else {
			float prev_time = record->release_time;
			Vector2f pos_delta = get_pos_delta(record->orbit_pos, cursor_pos, record->window_height);
			record->is_orbiting = 0;
			record->orbit_delta = record->orbit_delta + pos_delta;
			if (prev_time && curr_time - prev_time < CLICK_DELAY) {
				record->double_click = 1;
				record->release_time = 0;
			}
			else {
				record->release_time = curr_time;
				record->release_pos = cursor_pos;
			}
		}
	}
	else if (button == BUTTON_R) {
		if (pressed) {
			record->is_panning = 1;
			record->pan_pos = cursor_pos;
		}
		else {
			Vector2f pos_delta = get_pos_delta(record->pan_pos, cursor_pos, record->window_height);
			record->is_panning = 0;
			record->pan_delta = record->pan_delta + pos_delta;
		}
	}
}

void update_camera(window_t* window, Camera* camera,
	Record* record) {
	Vector2f cursor_pos = get_cursor_pos(window);
	if (record->is_orbiting) {
		Vector2f pos_delta = get_pos_delta(record->orbit_pos, cursor_pos, record->window_height);
		record->orbit_delta = record->orbit_delta + pos_delta;
		record->orbit_pos = cursor_pos;
	}
	if (record->is_panning) {
		Vector2f pos_delta = get_pos_delta(record->pan_pos, cursor_pos, record->window_height);
		record->pan_delta = record->pan_delta + pos_delta;
		record->pan_pos = cursor_pos;
	}
	if (input_key_pressed(window, KEY_SPACE)) {
		camera->set_transform(CAMERA_POSITION, CAMERA_TARGET);
	}
	else {
		Motion motion;
		motion.orbit = record->orbit_delta;
		motion.pan = record->pan_delta;
		motion.dolly = record->dolly_delta;
		camera->update_transform(motion);
	}
}
