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

		void Focus(glm::vec2 focalPoint);
		void Move(glm::vec2 deltaPos);

		const CameraData& GetCameraData();

	private:
		CameraData m_Data{};

		float m_AspectRatio{ 1.f };
		float m_Zoom{ 1.f };
		glm::vec2 m_FocalPoint{ 0.f, 0.f };

		bool m_UpdateCameraData{ true };
	};
}

#endif
