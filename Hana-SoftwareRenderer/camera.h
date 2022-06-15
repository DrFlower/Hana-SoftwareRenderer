#ifndef CAMERA_H
#define CAMERA_H

#include "math.h"

typedef struct camera camera_t;
typedef struct { Vector2f orbit; Vector2f pan; float dolly;} motion_t;

/* camera creating/releasing */
camera_t *camera_create(Vector3f position, Vector3f target, float aspect);
void camera_release(camera_t *camera);

/* camera updating */
void camera_set_transform(camera_t *camera, Vector3f position, Vector3f target);
void camera_update_transform(camera_t *camera, motion_t motion);

/* property retrieving */
Vector3f camera_get_position(camera_t *camera);
Vector3f camera_get_forward(camera_t *camera);
Matrix4x4 camera_get_view_matrix(camera_t *camera);
Matrix4x4 camera_get_proj_matrix(camera_t *camera);

#endif
