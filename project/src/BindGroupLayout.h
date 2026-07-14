#ifndef BINDGROUP_LAYOUT
#define BINDGROUP_LAYOUT

#include <unordered_map>
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

		template<typename T>
		bool HasUniformEntry(int binding);

		void ConfirmLayout(const GraphicsContext& context);
		bool IsLocked() const { return m_BindGroupLayout != nullptr; }

		WGPUBindGroupLayout GetLayout() const { return m_BindGroupLayout; }
		const GraphicsContext* GetGraphicsContext() const { return m_Context; }

	private:
		WGPUShaderStage GetShaderStage(BindingVisibility visibility);

		WGPUBindGroupLayout m_BindGroupLayout{ nullptr };

		std::unordered_map<int, WGPUBindGroupLayoutEntry> m_Entries{};
		const GraphicsContext* m_Context{ nullptr };
	};

	// could be moved to a .inl file, which is a type of header file for inline functions
	template<typename T>
	inline bool BindGroupLayout::AddUniformEntry(int binding, BindingVisibility visibility)
	{
		if (IsLocked()) return false;
		if (!m_Entries.contains(binding)) return false;

		auto [itr, inserted] = m_Entries.emplace(binding, WGPUBindGroupLayoutEntry{});

		if (!inserted) return false;

		auto& newEntry = itr->second;

		newEntry.binding = binding;
		newEntry.visibility = GetShaderStage(visibility);
		newEntry.buffer.type = WGPUBufferBindingType_Uniform;
		newEntry.buffer.minBindingSize = sizeof(T);

		return true;
	}

	template<typename T>
	inline bool BindGroupLayout::HasUniformEntry(int binding)
	{
		if (!m_Entries.contains(binding)) return false;

		auto itr = m_Entries.find(binding);

		return itr->second.buffer.minBindingSize == sizeof(T);
	}
}

#endif
