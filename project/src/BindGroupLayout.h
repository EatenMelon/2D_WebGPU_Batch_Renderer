#ifndef BINDGROUP_LAYOUT
#define BINDGROUP_LAYOUT

#include <unordered_map>
#include <webgpu/webgpu.h>

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

		void ClearEntries();
		void RemoveEntry(int binding);

	private:
		WGPUShaderStage GetShaderStage(BindingVisibility visibility);

		std::unordered_map<int, WGPUBindGroupLayoutEntry> m_Entries{};

	};

	// could be moved to a .inl file, which is a type of header file for inline functions
	template<typename T>
	inline bool BindGroupLayout::AddUniformEntry(int binding, BindingVisibility visibility)
	{
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
}

#endif
