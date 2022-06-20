#include "input.h"

Vector2f get_pos_delta(Vector2f old_pos, Vector2f new_pos) {
	Vector2f delta = new_pos - old_pos;
	return delta / (float)WINDOW_HEIGHT_TMP;
}

Vector2f get_cursor_pos(window_t* window) {
	float xpos, ypos;
	input_query_cursor(window, &xpos, &ypos);
	return Vector2f(xpos, ypos);
}

void scroll_callback(window_t* window, float offset) {
	record_t* record = (record_t*)window_get_userdata(window);
	record->dolly_delta += offset;
}

void button_callback(window_t* window, button_t button, int pressed) {
	record_t* record = (record_t*)window_get_userdata(window);
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
			Vector2f pos_delta = get_pos_delta(record->orbit_pos, cursor_pos);
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
			Vector2f pos_delta = get_pos_delta(record->pan_pos, cursor_pos);
			record->is_panning = 0;
			record->pan_delta = record->pan_delta + pos_delta;
		}
	}
}

void update_camera(window_t* window, Camera* camera,
	record_t* record) {
	Vector2f cursor_pos = get_cursor_pos(window);
	if (record->is_orbiting) {
		Vector2f pos_delta = get_pos_delta(record->orbit_pos, cursor_pos);
		record->orbit_delta = record->orbit_delta + pos_delta;
		record->orbit_pos = cursor_pos;
	}
	if (record->is_panning) {
		Vector2f pos_delta = get_pos_delta(record->pan_pos, cursor_pos);
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
