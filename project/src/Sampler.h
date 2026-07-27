#ifndef _SAMPLER
#define _SAMPLER

#include "GraphicsContext.h"

namespace wgpu
{
	enum class AddressMode { Repeat, MirrorRepeat, Clamp };
	enum class FilterMode { Linear, Nearest };

	class Sampler final
	{
	public:
		enum class Preset { PixelArt, Smooth };

		Sampler(const GraphicsContext& context, Preset preset);
		Sampler(const GraphicsContext& context, AddressMode u, AddressMode v, FilterMode mag, FilterMode min);

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

		Sampler(const GraphicsContext& context, SamplerSettings settings);

		static SamplerSettings GetSettings(Preset preset);

		static WGPUAddressMode GetWGPUAddressMode(AddressMode mode);
		static WGPUFilterMode GetFilterMode(FilterMode mode);

		WGPUSampler m_Sampler{ nullptr };

		const GraphicsContext* m_Context{ nullptr };
	};
}

#endif
