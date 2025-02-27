#pragma once
#include "Constants.h"

namespace ducklib::Render
{
struct Buffer
{
	uint32 size;
	Format format;
	void* apiResource;
	void* apiDescriptor;
};
}
