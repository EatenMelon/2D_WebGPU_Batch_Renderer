#ifndef MATERIAL
#define MATERIAL

#include <any>
#include <functional>

#include "Pipeline.h"

namespace wgpu
{
	class Material
	{
	public:
		Material(const Pipeline& pipeline);

		template<typename T>
		bool SetUniform(int binding, T value);
		bool SetTexture(int binding, const Texture2D* texture);

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

		const Pipeline* m_Pipeline{ nullptr };
	};

	template<typename T>
	inline bool Material::SetUniform(int binding, T value)
	{
		auto layout = m_Pipeline->GetBindGroupLayout();

		if (!layout->HasUniformEntry<T>()) return false;
		
		m_Uniform = CreateUniform(binding, value);
		m_UpdateUniformBuffer = true;

		return true;
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
