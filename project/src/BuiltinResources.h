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
	class Sampler;

	class BuiltinResources final
	{
	public:
		BuiltinResources(const Renderer2D& renderer);
		~BuiltinResources() noexcept;

		BuiltinResources(const BuiltinResources&) = delete;
		BuiltinResources& operator=(const BuiltinResources&) = delete;
		BuiltinResources(BuiltinResources&&) = delete;
		BuiltinResources& operator=(BuiltinResources&&) = delete;

		Material* GetSolidColorMaterial() const;
		std::unique_ptr<Material> CreateTextureMaterial() const;

		Sampler* GetNearestSampler() const;
		Sampler* GetLinearSampler() const;

	private:
		const Renderer2D* m_Renderer{ nullptr };

		std::unique_ptr<Shader> m_SolidColorShader{ nullptr };
		std::unique_ptr<BindGroupLayout> m_SolidColorLayout{ nullptr };
		std::unique_ptr<Pipeline> m_SolidColorPipeline{ nullptr };
		std::unique_ptr<Material> m_SolidColorMaterial{ nullptr };

		std::unique_ptr<Shader> m_TextureShader{ nullptr };
		std::unique_ptr<BindGroupLayout> m_TextureLayout{ nullptr };
		std::unique_ptr<Pipeline> m_TexturePipeline{ nullptr };

		std::unique_ptr<Sampler> m_NearestSampler{ nullptr };
		std::unique_ptr<Sampler> m_LinearSampler{ nullptr };

		static const std::string_view m_SolidColorSource;
		static const std::string_view m_TextureSource;
	};
}

#endif
