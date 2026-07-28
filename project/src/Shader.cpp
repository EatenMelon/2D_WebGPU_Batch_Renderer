#include "Shader.h"
#include <fstream>

wgpu::Shader::Shader(const Renderer2D& renderer, const std::filesystem::path& path)
    : m_Renderer{ &renderer }
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open " + path.filename().string() + "!");
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string shaderSource(size, ' ');
    file.seekg(0);
    file.read(shaderSource.data(), size);

    WGPUShaderSourceWGSL shaderCodeDesc{};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
    shaderCodeDesc.code = WGPUStringView(shaderSource.c_str(), shaderSource.size());

    WGPUShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain = &shaderCodeDesc.chain;

    m_ShaderModule = wgpuDeviceCreateShaderModule(m_Renderer->GetContext()->GetDevice(), &shaderDesc);
}

wgpu::Shader::~Shader() noexcept
{
    wgpuShaderModuleRelease(m_ShaderModule);
}
