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

	enum class BindingVisibility
	{
		VertexShaderStage,
		FragmentShaderStage,
		Both,
		None
	};

	class BindGroupLayout final
	{
	public:

		template<typename T>
		bool AddUniformEntry(int binding, BindingVisibility visibility);
		bool AddTextureEntry(int binding);
		bool AddSamplerEntry(int binding);

		// used only for post processing
		bool AddFrameEntry(int bindng);

		template<typename T>
		int GetUniformEntryBinding() const;
		bool RequiresUniform() const;

		int GetFrameEntryBinding() const;
		
		// this locks down the BindgroupLayout, 
		// to make it ready for use and making it immutable
		void ConfirmLayout(const Renderer2D& renderer);
		bool IsLocked() const { return m_BindGroupLayout != nullptr; }

		uint64_t GetRequiredUniformBufferSize() const;

		WGPUBindGroupLayout GetLayout() const { return m_BindGroupLayout; }
		const GraphicsContext* GetContext() const { return m_Context; }

	private:
		WGPUShaderStage GetShaderStage(BindingVisibility visibility);

		struct UniformEntry
		{
			int location{};
			uint64_t size{};
			std::type_index typeIndex{ typeid(void*) };
		};

		WGPUBindGroupLayout m_BindGroupLayout{ nullptr };

		std::vector<UniformEntry> m_UniformEntries{};
		uint64_t m_UniformBufferSize{ 0 };
		std::unordered_map<int, WGPUBindGroupLayoutEntry> m_Entries{};
		std::unique_ptr<WGPUBindGroupLayoutEntry> m_FrameEntry{ nullptr };

		const GraphicsContext* m_Context{ nullptr };
	};

	// could be moved to a .inl file, which is a type of header file for inline functions
	template<typename T>
	inline bool BindGroupLayout::AddUniformEntry(int binding, [[maybe_unused]] BindingVisibility visibility)
	{
		if (IsLocked()) return false;

		UniformEntry newEntry;
		newEntry.location = binding;
		newEntry.size = sizeof(T);
		newEntry.typeIndex = typeid(T);
		
		m_UniformEntries.push_back(newEntry);

		return true;
	}

	template<typename T>
	inline int BindGroupLayout::GetUniformEntryBinding() const
	{
		if (m_UniformEntries.size() <= 0) return -1;

		return 0;
	}

	
}

#endif
