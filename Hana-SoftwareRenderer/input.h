#pragma once

#include "api.h"
#include "platform.h"
#include "camera.h"

static const Vector3f CAMERA_POSITION = { 0, 0, 2.f };
static const Vector3f CAMERA_TARGET = { 0, 0, 0 };

static const float CLICK_DELAY = 0.25f;
static const int WINDOW_HEIGHT_TMP = 600;

class record_t {
public:
	/* orbit */
	int is_orbiting;
	Vector2f orbit_pos;
	Vector2f orbit_delta;
	/* pan */
	int is_panning;
	Vector2f pan_pos;
	Vector2f pan_delta;
	/* zoom */
	float dolly_delta;
	/* light */
	float light_theta;
	float light_phi;
	/* click */
	float press_time;
	float release_time;
	Vector2f press_pos;
	Vector2f release_pos;
	int single_click;
	int double_click;
	Vector2f click_pos;
};

Vector2f get_pos_delta(Vector2f old_pos, Vector2f new_pos);

Vector2f get_cursor_pos(window_t* window);

void scroll_callback(window_t* window, float offset);
void button_callback(window_t* window, button_t button, int pressed);
void update_camera(window_t* window, Camera* camera, record_t* record);
