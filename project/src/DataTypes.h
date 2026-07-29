#ifndef DATATYPES
#define DATATYPES

#include <glm/glm.hpp>

namespace wgpu
{

	// only works for MSVC
	// but otherwise i get a unnamed struct/union warning
#pragma warning(push)
#pragma warning(disable : 4201)

	// just need it to for naming purposes, could just be a vec4
	struct ColorF
	{
		ColorF(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.f);

		union
		{
			struct
			{
				float r;
				float g;
				float b;
				float a;
			};

			glm::vec4 vec;
		};
	};

#pragma warning(pop)

	struct Vertex
	{
		glm::vec3 position{};
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
