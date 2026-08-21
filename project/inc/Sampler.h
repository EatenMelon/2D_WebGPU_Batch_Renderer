#ifndef _SAMPLER
#define _SAMPLER

#include <webgpu/webgpu.h>

namespace wgpu
{
	class Renderer2D;
	class Canvas;
	class GraphicsContext;

	enum class AddressMode { Repeat, MirrorRepeat, Clamp };
	enum class FilterMode { Linear, Nearest };

	class Sampler final
	{
	public:
		enum class Preset { Nearest, Linear };

		Sampler(const Renderer2D& renderer, Preset preset);
		Sampler(const Renderer2D& renderer, AddressMode u, AddressMode v, FilterMode mag, FilterMode min);

		Sampler(const Canvas& canvas, Preset preset);
		Sampler(const Canvas& canvas, AddressMode u, AddressMode v, FilterMode mag, FilterMode min);

		~Sampler() noexcept;
		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;
		Sampler(Sampler&&) = delete;
		Sampler& operator=(Sampler&&) = delete;

		WGPUSampler GetSampler() const { return m_Sampler; }

	private:
		struct SamplerSettings
		{
			AddressMode addressModeU{};
			AddressMode addressModeV{};
			FilterMode magFilter{};
			FilterMode minFilter{};
		};

		Sampler(const Renderer2D& renderer, SamplerSettings settings);

		static SamplerSettings GetSettings(Preset preset);

		static WGPUAddressMode GetWGPUAddressMode(AddressMode mode);
		static WGPUFilterMode GetFilterMode(FilterMode mode);

		WGPUSampler m_Sampler{ nullptr };

		const GraphicsContext* m_Context{ nullptr };
	};
}

#endif
