#ifndef MATERIAL
#define MATERIAL

#include <any>
#include <functional>

#include "Pipeline.h"
#include "Uniforms.h"

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
		bool SetUniformVariable(size_t location, const T& value);

		bool SetTexture(int binding, const Texture2D* texture);
		bool SetSampler(int binding, const Sampler* sampler);
		bool SetFrame(int binding, WGPUTextureView frameTextureView);

		int GetUniformBinding() const;

		template<typename T>
		int GetUniformVariableLocation() const;

		WGPUBindGroup GetBindGroup();
		const Pipeline* GetPipeline() const { return m_Pipeline; }

	private:
		void UpdateUniformBuffer();
		void UpdateBindgroup();

		WGPUBindGroup m_BindGroup{ nullptr };
		WGPUBuffer m_UniformBuffer{ nullptr };
		bool m_UpdateBindGroup{ true };
		bool m_UpdateUniformBuffer{ true };

		std::vector<std::unique_ptr<IUniform>> m_Uniform{};
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

	/// <summary>
	/// Changes the value of a variable in the uniform of the shader used by this material
	/// </summary>
	/// <param name="location"> 
	/// The indexed location of the variable, similar to how elements in an array are stored
	/// </param>
	/// <param name="value">
	/// The new value for the uniform variable
	/// </param>
	/// <returns> True if the variable has been added successfully </returns>
	template<typename T>
	inline bool Material::SetUniformVariable(size_t location, const T& value)
	{
		if (m_Uniform.empty()) return false;

		const auto layout = m_Pipeline->GetBindGroupLayout();

		if (layout->GetUniformCount() == 0) return false;

		auto& base = m_Uniform[location];

		if (base == nullptr)
		{
			base = std::make_unique<TypedUniform<T>>();
		}

		auto typed = static_cast<TypedUniform<T>*>(base.get());

		typed->SetValue(value);

		m_UpdateUniformBuffer = true;

		return true;
	}

	template<typename T>
	inline int Material::GetUniformVariableLocation() const
	{
		const auto layout = m_Pipeline->GetBindGroupLayout();

		if (layout->GetUniformCount() == 0) return -1;

		return layout->GetUniformVariableLocation<T>();
	}
}

#endif
