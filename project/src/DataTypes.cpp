#include <DataTypes.h>

wgpu::ColorF::ColorF(float r, float g, float b, float a)
	: r{ r }, g{ g }, b{ b }, a{ a }
{}

wgpu::RectF::RectF(float left, float bottom, float width, float height)
	: left{ left }
	, bottom{ bottom }
	, width{ width }
	, height{ height }
{}
