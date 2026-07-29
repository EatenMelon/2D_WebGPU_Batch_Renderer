#include "Camera2D.h"

#include <limits>
#include <glm/ext.hpp>

void wgpu::Camera2D::SetAspectRatio(float aspectRatio)
{
	if (aspectRatio <= 0.f)
	{
		m_AspectRatio = 1.f;
	}

	m_AspectRatio = aspectRatio;
	m_UpdateCameraData = true;
}

void wgpu::Camera2D::SetZoom(float zoom)
{
	m_Zoom = zoom;
	m_UpdateCameraData = true;

	if (m_Zoom <= 0.f)
	{
		m_Zoom = std::numeric_limits<float>::epsilon();
	}

}

void wgpu::Camera2D::Zoom(float deltaZoom)
{
	SetZoom(m_Zoom + deltaZoom);
}

void wgpu::Camera2D::Focus(glm::vec2 focalPoint)
{
	m_FocalPoint = focalPoint;
	m_UpdateCameraData = true;
}

void wgpu::Camera2D::Move(glm::vec2 deltaPos)
{
	m_FocalPoint += deltaPos;
	m_UpdateCameraData = true;
}

const wgpu::CameraData& wgpu::Camera2D::GetCameraData()
{
	if (!m_UpdateCameraData) return m_Data;

	const float viewHeight{ m_Zoom };
	const float viewWidth{ viewHeight * m_AspectRatio };

	// I use orthoZO because it suits webGPU's Zero to One depth range
	m_Data.projection = glm::orthoZO
	(
		-viewWidth / 2.f,
		viewWidth / 2.f,
		-viewHeight / 2.f,
		viewHeight / 2.f,
		-1.f,
		1.f
	);

	m_Data.view = glm::translate(glm::mat4(1.f), -glm::vec3(m_FocalPoint, 0.0f));
	m_UpdateCameraData = false;

	return m_Data;
}

