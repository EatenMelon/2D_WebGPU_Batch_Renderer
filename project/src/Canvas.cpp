#include "Canvas.h"

#include <vector>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer2D.h"
#include "BuiltinResources.h"
#include "Texture2D.h"

wgpu::Canvas::Canvas(SDL_Window* window)
	: m_Renderer{ std::make_unique<Renderer2D>(window) }
{

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

	auto material = m_Renderer->GetBuiltinResources()->GetSolidColorMaterial();
	RenderQuad(material, points[0], points[1], points[2], points[3]);
}

void wgpu::Canvas::FillRect(float left, float bottom, float width, float height) const
{
	const auto p0 = glm::vec2{ left, bottom + height };
	const auto p1 = glm::vec2{ left, bottom };
	const auto p2 = glm::vec2{ left + width, bottom };
	const auto p3 = glm::vec2{ left + width, bottom + height };

	auto material = m_Renderer->GetBuiltinResources()->GetSolidColorMaterial();
	RenderQuad(material, p0, p1, p2, p3);
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

			auto material = m_Renderer->GetBuiltinResources()->GetSolidColorMaterial();
			RenderQuad(material, p0, p1, p2, p3);
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

void wgpu::Canvas::DrawTexture(const Texture2D& texture, const RectF& dst) const
{
	DrawTexture(texture, dst, RectF{ 0.f, 0.f, texture.GetSize().x, texture.GetSize().y });
}

void wgpu::Canvas::DrawTexture(const Texture2D& texture, const RectF& dst, const RectF& src) const
{
	const auto cutout = texture.GetCutout(src);

	Vertex2D v0{};	// bottom-left
	v0.position = dst.pos;
	v0.uv = cutout.pos;
	v0.uv.y += cutout.size.y;
	v0.color = texture.GetColorMultiplier();

	Vertex2D v1{};	// bottom-right
	v1.position = dst.pos;
	v1.position.x += dst.size.x;
	v1.uv = cutout.pos + cutout.size;
	v1.color = texture.GetColorMultiplier();

	Vertex2D v2{};	// top-right
	v2.position = dst.pos + dst.size;
	v2.uv = cutout.pos;
	v2.uv.x += cutout.size.x;
	v2.color = texture.GetColorMultiplier();

	Vertex2D v3{};	// top-left
	v3.position = dst.pos;
	v3.position.y += dst.size.y;
	v3.uv = cutout.pos;
	v3.color = texture.GetColorMultiplier();

	RenderQuad(texture.GetMaterial(), v0, v1, v2, v3);
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

wgpu::Renderer2D* wgpu::Canvas::GetRenderer() const
{
	return m_Renderer.get();
}

void wgpu::Canvas::RenderQuad(Material* mat, const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) const
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

	m_Renderer->SubmitQuad(*mat, v0, v1, v2, v3);
}

void wgpu::Canvas::RenderQuad(Material* mat, const Vertex2D& p0, const Vertex2D& p1, const Vertex2D& p2, const Vertex2D& p3) const
{
	wgpu::Vertex3D v0{};
	v0.position = glm::vec3{ p0.position, m_DrawLayer };
	v0.color = p0.color;
	v0.uv = p0.uv;

	wgpu::Vertex3D v1{};
	v1.position = glm::vec3{ p1.position, m_DrawLayer };
	v1.color = p1.color;
	v1.uv = p1.uv;

	wgpu::Vertex3D v2{};
	v2.position = glm::vec3{ p2.position, m_DrawLayer };
	v2.color = p2.color;
	v2.uv = p2.uv;

	wgpu::Vertex3D v3{};
	v3.position = glm::vec3{ p3.position, m_DrawLayer };
	v3.color = p3.color;
	v3.uv = p3.uv;

	m_Renderer->SubmitQuad(*mat, v0, v1, v2, v3);
}
