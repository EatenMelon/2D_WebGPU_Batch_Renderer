#include "DataTypes.h"

wgpu::ColorF::ColorF(float r, float g, float b, float a)
	: r{ r }, g{ g }, b{ b }, a{ a }
{}

wgpu::RectF::RectF(const glm::vec2& pos, const glm::vec2& size)
	: position{ pos }
	, size{ size }
{}

wgpu::RectF::RectF(const glm::vec2 & pos, float size)
	: position{ pos }
	, size{ size, size }
{}

wgpu::EllipseF::EllipseF(const glm::vec2& center, const glm::vec2& radii)
	: center{ center }
	, radii{ radii }
{}

wgpu::EllipseF::EllipseF(const glm::vec2 & center, float radius)
	: center{ center }
	, radii{ radius, radius }
{}
