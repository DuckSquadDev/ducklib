#pragma once

namespace ducklib::Render
{
struct Viewport
{
	float topLeftX;
	float topLeftY;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};
}
