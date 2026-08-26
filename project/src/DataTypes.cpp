#include <DataTypes.h>

wgpu::ColorF::ColorF(float r, float g, float b, float a)
	: r{ r }, g{ g }, b{ b }, a{ a }
{}

wgpu::RectF::RectF(float left, float bottom, float width, float height)
	: pos{ left, bottom }
	, size{ width, height }
{}

wgpu::RectF::RectF(const glm::vec2& pos, const glm::vec2& size)
	: pos{ pos }
	, size{ size }
{}

wgpu::RectF::RectF(const glm::vec2 & pos, float size)
	: pos{ pos }
	, size{ size, size }
{}
