#ifndef _CANVAS
#define _CANVAS

#include <memory>
#include <string>

#include "DataTypes.h"

struct SDL_Window;

namespace wgpu
{
	class Renderer2D;

	class Canvas
	{
	public:
		Canvas(SDL_Window* window);

		void DrawRect(float left, float bottom, float width, float height);

		void SetDrawColor(const ColorF& color);
		void SetDrawLayer(float layer);

		ColorF GetDrawColor() const;
		float GetDrawLayer() const;

	private:
		ColorF m_DrawColor{ 1.f, 1.f, 1.f, 1.f };
		float m_DrawLayer{ 0.f };

		std::unique_ptr<Renderer2D> m_Renderer{ nullptr };

		static const std::string m_SolidColorShaderSource;
	};
}

#endif
