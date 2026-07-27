#ifndef BINDGROUP_LAYOUT
#define BINDGROUP_LAYOUT

#include <cstdint>
#include <unordered_map>
#include <optional>
#include <typeindex>

#include "GraphicsContext.h"

namespace wgpu
{
	enum class BindingVisibility
	{
		VertexShaderStage,
		FragmentShaderStage,
		Both,
		None
	};

	class BindGroupLayout
	{
	public:

		template<typename T>
		bool AddUniformEntry(int binding, BindingVisibility visibility);
		bool AddTextureEntry(int binding);
		bool AddSamplerEntry(int binding);

		template<typename T>
		bool HasUniformEntry() const;
		bool RequiresUniform() const;
		int GetUniformEntryBinding() const;

		void ConfirmLayout(const GraphicsContext& context);
		bool IsLocked() const { return m_BindGroupLayout != nullptr; }

		uint64_t GetRequiredUniformBufferSize() const;

		WGPUBindGroupLayout GetLayout() const { return m_BindGroupLayout; }
		const GraphicsContext* GetGraphicsContext() const { return m_Context; }

	private:
		WGPUShaderStage GetShaderStage(BindingVisibility visibility);

		WGPUBindGroupLayout m_BindGroupLayout{ nullptr };

		std::optional<std::pair<std::type_index, WGPUBindGroupLayoutEntry>> m_UniformEntry{};
		std::unordered_map<int, WGPUBindGroupLayoutEntry> m_Entries{};
		const GraphicsContext* m_Context{ nullptr };
	};

	// could be moved to a .inl file, which is a type of header file for inline functions
	template<typename T>
	inline bool BindGroupLayout::AddUniformEntry(int binding, BindingVisibility visibility)
	{
		if (IsLocked()) return false;
		
		WGPUBindGroupLayoutEntry entry{};

		entry.binding = binding;
		entry.visibility = GetShaderStage(visibility);
		entry.buffer.type = WGPUBufferBindingType_Uniform;
		entry.buffer.minBindingSize = sizeof(T);

		m_UniformEntry.emplace(typeid(T), entry);

		return true;
	}

	// not the best check but it is what it is
	template<typename T>
	inline bool BindGroupLayout::HasUniformEntry() const
	{
		if (!m_UniformEntry.has_value()) return false;

		return m_UniformEntry.value().first == typeid(T);
	}
}

#endif
