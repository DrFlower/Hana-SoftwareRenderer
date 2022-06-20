#ifndef CAMERA_H
#define CAMERA_H

#include "api.h"

struct Motion { Vector2f orbit; Vector2f pan; float dolly; };

class Camera
{
	Vector3f position;
	Vector3f target;
	float aspect;

public:
	Camera(Vector3f position, Vector3f target, float aspect);

	/* camera updating */
	void set_transform(Vector3f position, Vector3f target);
	void update_transform(Motion motion);

	/* property retrieving */
	Vector3f get_position();
	Vector3f get_forward();
	Matrix4x4 get_view_matrix();
	Matrix4x4 get_proj_matrix();
};

#endif
