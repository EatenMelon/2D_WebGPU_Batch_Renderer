#include "Canvas.h"
#include "Renderer2D.h"

wgpu::Canvas::Canvas(SDL_Window* window)
	: m_Renderer{ std::make_unique<Renderer2D>(window) }
{
	
}

void wgpu::Canvas::DrawRect(float left, float bottom, float width, float height)
{
	wgpu::Vertex3D topLeft{};
	topLeft.position = glm::vec3{ left, bottom + height, m_DrawLayer };
	topLeft.color = m_DrawColor;

	wgpu::Vertex3D topRight{};
	topRight.position = glm::vec3{ left + width, bottom + height, m_DrawLayer };
	topRight.color = m_DrawColor;

	wgpu::Vertex3D bottomRight{};
	bottomRight.position = glm::vec3{ left + width, bottom, m_DrawLayer };
	bottomRight.color = m_DrawColor;

	wgpu::Vertex3D bottomLeft{};
	bottomLeft.position = glm::vec3{ left, bottom, m_DrawLayer };
	bottomLeft.color = m_DrawColor;

	
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
