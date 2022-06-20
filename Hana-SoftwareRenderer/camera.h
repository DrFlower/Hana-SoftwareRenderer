#ifndef CAMERA_H
#define CAMERA_H

#include "math.h"

struct Motion { Vector2f orbit; Vector2f pan; float dolly; };

class Camera
{
	Vector3f position;
	Vector3f target;
	float aspect;

public:
	Camera(Vector3f position, Vector3f target, float aspect);

	/* camera creating/releasing */
	//camera_t* camera_create(Vector3f position, Vector3f target, float aspect);
	//void camera_release(camera_t* camera);

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
