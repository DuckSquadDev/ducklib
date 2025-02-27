#pragma once
#include "Constants.h"

namespace ducklib::Render
{
struct InputComponent
{
	char* name;
	uint32 index;
	Format format;
	uint32 inputSlot;
	uint32 alignOffset;
	uint32 instanceStepRate;
	bool isInstanceData;
};

struct InputDescription
{
	InputComponent* inputComponents;
	uint32 numInputComponents;
};
}
