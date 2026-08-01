#include "Canvas.h"
#include "Renderer2D.h"
#include "BuiltinResources.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

wgpu::Canvas::Canvas(SDL_Window* window)
	: m_Renderer{ std::make_unique<Renderer2D>(window) }
{
	m_BuiltinResources = std::make_unique<BuiltinResources>(*m_Renderer.get());
}

wgpu::Canvas::~Canvas() noexcept = default;

void wgpu::Canvas::BeginFrame() const
{
	m_Renderer->BeginFrame();
}

void wgpu::Canvas::EndFrame() const
{
	m_Renderer->EndFrame();
	m_Renderer->Render();
}

void wgpu::Canvas::SetClearColor(const ColorF& color)
{
	m_Renderer->SetClearColor(color);
}

void wgpu::Canvas::SetCamera(const std::shared_ptr<Camera2D>& camera)
{
	m_Renderer->SetCamera(camera);
}

void wgpu::Canvas::Resize()
{
	m_Renderer->Resize();
}

void wgpu::Canvas::DrawLine(const glm::vec2& start, const glm::vec2& end, float lineWidth) const
{
	const glm::vec2 diff{ end - start };
	const float angle = atan2f(diff.y, diff.x);

	const glm::vec2 center{ (start + end) / 2.f };
	auto transform = glm::mat4(1.f);

	transform = glm::translate(transform, glm::vec3(center, m_DrawLayer));
	transform = glm::rotate(transform, angle, glm::vec3(0.f, 0.f, 1.f));
	transform = glm::scale(transform, glm::vec3(glm::length(diff), lineWidth, 1.f));

	std::vector<glm::vec2> points{};
	points.reserve(4);

	points.emplace_back(-0.5f, 0.5f);	// top-left
	points.emplace_back(-0.5f, -0.5f);	// bottom-left
	points.emplace_back(0.5f, -0.5f);	// bottom-right
	points.emplace_back(0.5f, 0.5f);	// top-right

	for (auto& p : points)
	{
		const glm::vec4 result = transform * glm::vec4(p, 0.f, 1.f);
		p = glm::vec2(result.x, result.y);
	}

	RenderQuad(points[0], points[1], points[2], points[3]);
}

void wgpu::Canvas::FillRect(float left, float bottom, float width, float height) const
{
	const auto p0 = glm::vec2{ left, bottom + height };
	const auto p1 = glm::vec2{ left, bottom };
	const auto p2 = glm::vec2{ left + width, bottom };
	const auto p3 = glm::vec2{ left + width, bottom + height };

	RenderQuad(p0, p1, p2, p3);
}

void wgpu::Canvas::FillRect(const RectF& rect) const
{
	FillRect(rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
}

void wgpu::Canvas::DrawRect(float left, float bottom, float width, float height, float lineWidth) const
{
	const glm::vec2 inner{ left + lineWidth / 2.f, bottom + lineWidth / 2.f };
	const glm::vec2 outer{ left - lineWidth / 2.f, bottom - lineWidth / 2.f };

	FillRect(outer.x, outer.y, lineWidth, height + lineWidth);
	FillRect(outer.x + width, outer.y, lineWidth, height + lineWidth);

	FillRect(outer.x, outer.y, width + lineWidth, lineWidth);
	FillRect(outer.x, outer.y + height, width + lineWidth, lineWidth);
}

void wgpu::Canvas::DrawRect(const RectF & rect, float lineWidth) const
{
	DrawRect(rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, lineWidth);
}

void wgpu::Canvas::FillEllipse(float x, float y, float xRadius, float yRadius) const
{
	auto getPoint = [&](float angle) -> glm::vec2
		{
			return glm::vec2
			{
				x + xRadius * cosf(angle),
				y + yRadius * sinf(angle)
			};
		};

	auto draw = [&](float angleA, float angleB) -> void
		{
			auto p0 = getPoint(angleA);
			auto p1 = getPoint(glm::pi<float>() - angleA);
			auto p2 = getPoint(glm::pi<float>() - angleB);
			auto p3 = getPoint(angleB);

			RenderQuad(p0, p1, p2, p3);
		};

	const float zoom{ GetCamera()->GetZoom() };

	constexpr float quarter{ glm::pi<float>() / 2 };
	constexpr float minIncr{ quarter / 15.f };
	constexpr float maxIncr{ quarter / 2.f };

	const float increment{ glm::clamp(zoom / 5.f, minIncr, maxIncr) };

	// an ellipse draws at least 4 triangles (12 vertices), and at most 56 triangles (168 vertices)
	for (float angle{ increment }; angle < quarter; angle += increment)
	{
		const float prevAngle{ angle - increment };
		draw(angle, prevAngle);
		draw(-prevAngle, -angle);
	}
}

void wgpu::Canvas::FillEllipse(const EllipseF& ellipse) const
{
	FillEllipse(ellipse.center.x, ellipse.center.y, ellipse.radii.x, ellipse.radii.y);
}

void wgpu::Canvas::SetDrawColor(const ColorF& color)
{
	m_DrawColor = color;
}

void wgpu::Canvas::SetDrawLayer(float layer)
{
	m_DrawLayer = layer;
}

wgpu::ColorF wgpu::Canvas::GetDrawColor() const
{
	return m_DrawColor;
}

float wgpu::Canvas::GetDrawLayer() const
{
	return m_DrawLayer;
}

std::shared_ptr<wgpu::Camera2D> wgpu::Canvas::GetCamera() const
{
	return m_Renderer->GetCamera();
}

void wgpu::Canvas::RenderQuad(const glm::vec2 p0, const glm::vec2 p1, const glm::vec2 p2, const glm::vec2 p3) const
{
	wgpu::Vertex3D v0{};
	v0.position = glm::vec3{ p0, m_DrawLayer };
	v0.color = m_DrawColor;

	wgpu::Vertex3D v1{};
	v1.position = glm::vec3{ p1, m_DrawLayer };
	v1.color = m_DrawColor;

	wgpu::Vertex3D v2{};
	v2.position = glm::vec3{ p2, m_DrawLayer };
	v2.color = m_DrawColor;

	wgpu::Vertex3D v3{};
	v3.position = glm::vec3{ p3, m_DrawLayer };
	v3.color = m_DrawColor;

	auto material = m_BuiltinResources->GetMaterial(BuiltinResources::Type::SolidColor);
	m_Renderer->SubmitQuad(*material, v0, v1, v2, v3);
}
