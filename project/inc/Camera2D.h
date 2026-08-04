#ifndef _CAMERA_2D
#define _CAMERA_2D

#include "DataTypes.h"

namespace wgpu
{
	class Camera2D final
	{
	public:
		void SetAspectRatio(float aspectRatio);

		void SetZoom(float zoom);
		void Zoom(float deltaZoom);

		void Focus(const glm::vec2& focalPoint);
		void Move(const glm::vec2& deltaPos);

		const CameraData& GetCameraData();
		float GetAspectRatio() const { return m_AspectRatio; }
		float GetZoom() const { return m_Zoom; }
		glm::vec2 GetFocalPoint() const { return m_FocalPoint; }

	private:
		CameraData m_Data{};

		float m_AspectRatio{ 1.f };
		float m_Zoom{ 1.f };
		glm::vec2 m_FocalPoint{ 0.f, 0.f };

		bool m_UpdateCameraData{ true };
	};
}

#endif
