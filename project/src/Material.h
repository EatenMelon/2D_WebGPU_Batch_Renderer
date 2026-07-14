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

		WGPUBindGroup GetBindGroup();
		const Pipeline* GetPipeline() const { return m_Pipeline; }

	private:
		void UpdateUniformBuffer();
		void UpdateBindgroup();

		struct Uniform
		{
			std::any value{};
			size_t size{};
			int binding{};
			std::function<void* (std::any&)> GetData{};
		};

		template<typename T>
		Uniform CreateUniform(int binding, const T& value) const;
		void GetSortedUniforms(std::vector<Uniform>& out) const;

		WGPUBindGroup m_BindGroup{ nullptr };
		WGPUBuffer m_UniformBuffer{ nullptr };
		bool m_UpdateBindGroup{ true };
		bool m_UpdateUniformBuffer{ true };

		std::unordered_map<int, Uniform> m_Uniforms{};

		const Pipeline* m_Pipeline{ nullptr };
	};

	template<typename T>
	inline bool Material::SetUniform(int binding, T value)
	{
		auto layout = m_Pipeline->GetBindGroupLayout();

		if (!layout->HasUniformEntry<T>(binding))
		{
			return false;
		}

		//try
		//{
		//	std::any_cast<T>(/*uniform*/);
		//}
		//catch (const std::bad_any_cast& ex)
		//{
		//	return false;
		//}
		m_Uniforms.insert_or_assign(binding, CreateUniform(binding, value));
		m_UpdateUniformBuffer = true;

		return true;
	}

	template<typename T>
	inline Material::Uniform Material::CreateUniform(int binding, const T& value) const
	{
		Uniform uniform{};

		uniform.value = value;
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
