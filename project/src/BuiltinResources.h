#ifndef _BUILTIN_RESOURCES
#define _BUILTIN_RESOURCES

#include <string_view>
#include <memory>

namespace wgpu
{
	class Renderer2D;

	class Shader;
	class BindGroupLayout;
	class Pipeline;
	class Material;

	class BuiltinResources final
	{
	public:
		BuiltinResources(const Renderer2D& renderer);
		~BuiltinResources() noexcept;

		BuiltinResources(const BuiltinResources&) = delete;
		BuiltinResources& operator=(const BuiltinResources&) = delete;
		BuiltinResources(BuiltinResources&&) = delete;
		BuiltinResources& operator=(BuiltinResources&&) = delete;

		enum class Type { SolidColor, Texture };

		Material* GetMaterial(Type resourceType) const;

	private:
		const Renderer2D* m_Renderer{ nullptr };

		std::unique_ptr<Shader> m_SolidColorShader{ nullptr };
		std::unique_ptr<BindGroupLayout> m_SolidColorLayout{ nullptr };
		std::unique_ptr<Pipeline> m_SolidColorPipeline{ nullptr };
		std::unique_ptr<Material> m_SolidColorMaterial{ nullptr };

		static const std::string_view m_SolidColorSource;
	};
}

#endif
