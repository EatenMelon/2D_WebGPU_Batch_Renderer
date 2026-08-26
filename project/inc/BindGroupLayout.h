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
		typedef std::pair<std::type_index, WGPUBindGroupLayoutEntry> UniformEntry;

		WGPUShaderStage GetShaderStage(BindingVisibility visibility);

		WGPUBindGroupLayout m_BindGroupLayout{ nullptr };

		std::unique_ptr<UniformEntry> m_UniformEntry{ nullptr };
		std::unordered_map<int, WGPUBindGroupLayoutEntry> m_Entries{};
		std::unique_ptr<WGPUBindGroupLayoutEntry> m_FrameEntry{ nullptr };

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

		m_UniformEntry = std::make_unique<UniformEntry>(typeid(T), entry);

		return true;
	}

	template<typename T>
	inline int BindGroupLayout::GetUniformEntryBinding() const
	{
		if (m_UniformEntry == nullptr) return -1;
		if (m_UniformEntry->first != typeid(T)) return -1;

		return m_UniformEntry->second.binding;
	}

	
}

#endif
