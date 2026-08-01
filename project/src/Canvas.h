#ifndef _CANVAS
#define _CANVAS

#include <memory>
#include <string>

#include "DataTypes.h"

struct SDL_Window;

namespace wgpu
{
	class Renderer2D;
	class Camera2D;
	class BuiltinResources;

	class Canvas final
	{
	public:
		Canvas(SDL_Window* window);
		~Canvas() noexcept;

		Canvas(const Canvas&) = delete;
		Canvas& operator=(const Canvas&) = delete;
		Canvas(Canvas&&) = delete;
		Canvas& operator=(Canvas&&) = delete;

		void BeginFrame();
		void EndFrame();

		void SetClearColor(const ColorF& color);
		void SetCamera(const std::shared_ptr<Camera2D>& camera);
		void Resize();

		void DrawLine(const glm::vec2& start, const glm::vec2& end, float lineWidth = 0.01f);

		void FillRect(float left, float bottom, float width, float height);
		void FillRect(const RectF& rect);
		void DrawRect(float left, float bottom, float width, float height, float lineWidth = 0.01f);
		void DrawRect(const RectF& rect, float lineWidth = 0.01f);

		void SetDrawColor(const ColorF& color);
		void SetDrawLayer(float layer);

		ColorF GetDrawColor() const;
		float GetDrawLayer() const;

	private:
		void DrawQuad(const glm::vec2 p0, const glm::vec2 p1, const glm::vec2 p2, const glm::vec2 p3);

		ColorF m_DrawColor{ 1.f, 1.f, 1.f, 1.f };
		float m_DrawLayer{ 0.f };

		std::unique_ptr<Renderer2D> m_Renderer{ nullptr };
		std::unique_ptr<BuiltinResources> m_BuiltinResources{ nullptr };

		static const std::string m_SolidColorShaderSource;
	};
}

#endif
