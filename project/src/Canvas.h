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

		void BeginFrame() const;
		void EndFrame() const;

		void SetClearColor(const ColorF& color);
		void SetCamera(const std::shared_ptr<Camera2D>& camera);
		void Resize();

		void DrawLine(const glm::vec2& start, const glm::vec2& end, float lineWidth = 0.01f) const;

		void FillRect(float left, float bottom, float width, float height) const;
		void FillRect(const RectF& rect) const;
		void DrawRect(float left, float bottom, float width, float height, float lineWidth = 0.01f) const;
		void DrawRect(const RectF& rect, float lineWidth = 0.01f) const;

		void FillEllipse(float x, float y, float xRadius, float yRadius) const;
		void FillEllipse(const EllipseF& ellipse) const;

		void SetDrawColor(const ColorF& color);
		void SetDrawLayer(float layer);

		ColorF GetDrawColor() const;
		float GetDrawLayer() const;

		std::shared_ptr<Camera2D> GetCamera() const;

	private:
		void RenderQuad(const glm::vec2 p0, const glm::vec2 p1, const glm::vec2 p2, const glm::vec2 p3) const;

		ColorF m_DrawColor{ 1.f, 1.f, 1.f, 1.f };
		float m_DrawLayer{ 0.f };

		std::unique_ptr<Renderer2D> m_Renderer{ nullptr };
		std::unique_ptr<BuiltinResources> m_BuiltinResources{ nullptr };

		static const std::string m_SolidColorShaderSource;
	};
}

#endif
