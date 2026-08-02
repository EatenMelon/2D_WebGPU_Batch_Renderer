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
	class Material;
	class Texture2D;

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
		void DrawEllipse(float x, float y, float xRadius, float yRadius, float lineWidth = 0.01f) const;
		void DrawEllipse(const EllipseF& ellipse, float lineWidth = 0.01f) const;

		void DrawTexture(const Texture2D& texture, const RectF& dst) const;
		void DrawTexture(const Texture2D& texture, const RectF& dst, const RectF& src) const;

		void SetDrawColor(const ColorF& color);
		void SetDrawLayer(float layer);

		ColorF GetDrawColor() const;
		float GetDrawLayer() const;

		std::shared_ptr<Camera2D> GetCamera() const;
		Renderer2D* GetRenderer() const;

	private:
		void RenderQuad(Material* mat, const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) const;
		void RenderQuad(Material* mat, const Vertex2D& p0, const Vertex2D& p1, const Vertex2D& p2, const Vertex2D& p3) const;

		glm::vec2 GetPointOnEllipse(float angle, const glm::vec2& pos, const glm::vec2& radii) const;

		ColorF m_DrawColor{ 1.f, 1.f, 1.f, 1.f };
		float m_DrawLayer{ 0.f };

		std::unique_ptr<Renderer2D> m_Renderer{ nullptr };

		static const std::string m_SolidColorShaderSource;
	};
}

#endif
