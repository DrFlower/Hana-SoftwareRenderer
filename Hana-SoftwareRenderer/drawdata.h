#pragma once

#include "model.h"
#include "IShader.h"
#include "renderbuffer.h"
#include "camera.h"
#include "gameobject.h"

struct DrawData {
	Model* model;
	IShader* shader;
	RenderBuffer* renderbuffer;
};
