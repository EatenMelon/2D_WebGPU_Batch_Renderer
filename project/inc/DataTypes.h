#ifndef DATATYPES
#define DATATYPES

#include <glm/glm.hpp>

namespace wgpu
{
	struct ColorF
	{
		ColorF() = default;
		ColorF(float r, float g, float b, float a = 1.f);

		float r{ 0.f };
		float g{ 0.f };
		float b{ 0.f };
		float a{ 1.f };
	};

	struct RectF
	{
		RectF() = default;
		RectF(float left, float bottom, float width, float height);
		RectF(const glm::vec2& pos, const glm::vec2& size);
		RectF(const glm::vec2& pos, float size);

		glm::vec2 pos{};
		glm::vec2 size{};
	};

	struct EllipseF
	{
		EllipseF() = default;
		EllipseF(float x, float y, float xRadius, float yRadius);
		EllipseF(const glm::vec2& center, const glm::vec2& radii);
		EllipseF(const glm::vec2& center, float radius);

		glm::vec2 center{};
		glm::vec2 radii{};
	};

	struct Vertex3D
	{
		glm::vec3 position{};
		ColorF color{};
		glm::vec2 uv{};
	};

	struct Vertex2D
	{
		glm::vec2 position{};
		ColorF color{};
		glm::vec2 uv{};
	};

	struct CameraData
	{
		glm::mat4x4 projection{ glm::mat4(1.f) };
		glm::mat4x4 view{ glm::mat4(1.f) };
	};
}


#endif
