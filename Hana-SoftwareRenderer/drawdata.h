#pragma once

#include "model.h"
#include "IShader.h"
#include "renderbuffer.h"

struct DrawData
{
	Model* model;
	IShader* shader;
	RenderBuffer* renderbuffer;
};