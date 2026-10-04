#ifndef BINDGROUP_LAYOUT
#define BINDGROUP_LAYOUT

#include <cstdint>
#include <unordered_map>
#include <typeindex>
#include <memory>

#include <wgpu.h>

namespace wgpu
{
	class Renderer2D;
	class GraphicsContext;

	class BindGroupLayout final
	{
	public:
		// Sets the number of variables your uniform will use.
		void SetUniformCount(size_t count);

		template<typename T>
		bool AddUniformVariable(size_t location);

		bool AddTextureEntry(int binding);
		bool AddSamplerEntry(int binding);

		// used only for post processing
		bool AddFrameEntry(int bindng);

		int GetFrameEntryBinding() const;
		
		// This locks down the BindGroupLayout, to make it ready for use.
		// This also causes this BindGroupLayout to be locked afterwards, making it immutable.
		void ConfirmLayout(const Renderer2D& renderer);
		bool IsLocked() const { return m_BindGroupLayout != nullptr; }

		template<typename T>
		int GetUniformVariableLocation() const;

		int GetUniformEntryBinding() const;
		size_t GetUniformCount() const;
		uint64_t GetUniformSize(size_t location) const;
		uint64_t GetRequiredUniformBufferSize() const;

		WGPUBindGroupLayout GetLayout() const { return m_BindGroupLayout; }
		const GraphicsContext* GetContext() const { return m_Context; }

	private:
		struct UniformEntry
		{
			size_t location{};
			uint64_t size{};
			std::type_index typeIndex{ typeid(void*) };
		};

		WGPUBindGroupLayout m_BindGroupLayout{ nullptr };

		const int m_UniformBinding{ 0 };
		uint64_t m_UniformBufferSize{ 0 };
		std::vector<UniformEntry> m_UniformEntries{};

		std::unordered_map<int, WGPUBindGroupLayoutEntry> m_Entries{};
		std::unique_ptr<WGPUBindGroupLayoutEntry> m_FrameEntry{ nullptr };

		const GraphicsContext* m_Context{ nullptr };
	};

	/// <summary>
	/// Adds a variable of the given type to the uniform (hasto match the shader for which it is used)
	/// </summary>
	/// <param name="location"> 
	/// The indexed location of the variable, similar to how elements in an array are stored
	/// </param>
	/// <returns> True if the variable has been added successfully </returns>
	template<typename T>
	inline bool BindGroupLayout::AddUniformVariable(size_t location)
	{
		if (IsLocked()) return false;

		UniformEntry newEntry;
		newEntry.location = location;
		newEntry.size = sizeof(T);
		newEntry.typeIndex = typeid(T);
		
		m_UniformEntries[location] = newEntry;

		return true;
	}

	template<typename T>
	inline int BindGroupLayout::GetUniformVariableLocation() const
	{
		if (m_UniformEntries.size() <= 0) return -1;
		
		const auto& typeIdx = typeid(T);

		for (size_t idx{ 0 }; idx < m_UniformEntries.size(); ++idx)
		{
			if (m_UniformEntries[idx].typeIndex != typeIdx) continue;

			return idx;
		}

		return -1;
	}
	
}

#endif
