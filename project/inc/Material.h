#ifndef MATERIAL
#define MATERIAL

#include <any>
#include <functional>

#include "Pipeline.h"

namespace wgpu
{
	class Texture2D;
	class Sampler;

	class Material
	{
	public:
		Material(const Pipeline& pipeline);
		~Material() noexcept;

		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;
		Material(Material&&) = delete;
		Material& operator=(Material&&) = delete;

		template<typename T>
		bool SetUniform(int binding, T value);
		bool SetTexture(int binding, const Texture2D* texture);
		bool SetSampler(int binding, const Sampler* sampler);
		bool SetFrame(int binding, WGPUTextureView frameTextureView);

		template<typename T>
		int GetUniformBinding();

		WGPUBindGroup GetBindGroup();
		const Pipeline* GetPipeline() const { return m_Pipeline; }

	private:
		void UpdateUniformBuffer();
		void UpdateBindgroup();

		struct Uniform
		{
			std::any any{};
			size_t size{};
			int binding{};
			std::function<void* (std::any&)> GetData{};
		};

		template<typename T>
		Uniform CreateUniform(int binding, const T& value) const;

		WGPUBindGroup m_BindGroup{ nullptr };
		WGPUBuffer m_UniformBuffer{ nullptr };
		bool m_UpdateBindGroup{ true };
		bool m_UpdateUniformBuffer{ true };

		Uniform m_Uniform{};
		std::unordered_map<int, const Texture2D*> m_Textures{};
		std::unordered_map<int, const Sampler*> m_Samplers{};

		struct Buffer
		{
			int binding{};
			WGPUTextureView textureView{ nullptr };
		};

		Buffer m_FrameBuffer{};

		const Pipeline* m_Pipeline{ nullptr };
	};

	template<typename T>
	inline bool Material::SetUniform(int binding, T value)
	{
		const auto layout = m_Pipeline->GetBindGroupLayout();
		const int requiredBinding = layout->GetUniformEntryBinding<T>();

		if (requiredBinding < 0) return false;
		if (binding != requiredBinding) return false;
		
		m_Uniform = CreateUniform(binding, value);
		m_UpdateUniformBuffer = true;

		return true;
	}

	template<typename T>
	inline int Material::GetUniformBinding()
	{
		auto layout = m_Pipeline->GetBindGroupLayout();

		if (layout == nullptr)
		{
			return -1;
		}

		return layout->GetUniformEntryBinding<T>();
	}

	template<typename T>
	inline Material::Uniform Material::CreateUniform(int binding, const T& value) const
	{
		Uniform uniform{};

		uniform.any = value;
		uniform.size = sizeof(T);
		uniform.binding = binding;

		uniform.GetData = [](std::any& value) -> void*
			{
				auto* ptr = std::any_cast<T>(&value);

				if (ptr != nullptr)
				{
					return static_cast<void*>(ptr);
				}

				return nullptr;
			};

		return uniform;
	}
}

#endif
