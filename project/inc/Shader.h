#ifndef _SHADER
#define _SHADER

#include <filesystem>
#include <wgpu.h>

namespace wgpu
{
	class Renderer2D;

	class Shader final
	{
	public:
		enum class ParsingMethod{ FromFile, FromString };

		Shader(const Renderer2D& renderer, const std::string& shader, ParsingMethod method = ParsingMethod::FromFile);
		~Shader() noexcept;

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&&) = delete;
		Shader& operator=(Shader&&) = delete;

		WGPUShaderModule GetShaderModule() const { return m_ShaderModule; }
		const Renderer2D* GetRenderer() const { return m_Renderer; }

	private:
		void LoadShaderFromSource(const std::string& shaderSource);

		WGPUShaderModule m_ShaderModule{ nullptr };

		const Renderer2D* m_Renderer{ nullptr };
	};
}

#endif
