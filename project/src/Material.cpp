#include <Material.h>

#include "Texture2D.h"
#include "Sampler.h"
#include "GraphicsContext.h"

wgpu::Material::Material(const Pipeline& pipeline)
	: m_Pipeline{ &pipeline }
{
	auto bindGroupLayout = m_Pipeline->GetBindGroupLayout();

	if (bindGroupLayout == nullptr) return;

    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.nextInChain = nullptr;
    bufferDesc.mappedAtCreation = false;
    bufferDesc.size = bindGroupLayout->GetRequiredUniformBufferSize();

    // Make sure to flag the buffer as BufferUsage::Uniform
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;

    m_UniformBuffer = wgpuDeviceCreateBuffer(m_Pipeline->GetContext()->GetDevice(), &bufferDesc);
}

wgpu::Material::~Material() noexcept
{
	if (m_BindGroup != nullptr)
	{
		wgpuBindGroupRelease(m_BindGroup);
		m_BindGroup = nullptr;
	}

	if (m_UniformBuffer != nullptr)
	{
		wgpuBufferRelease(m_UniformBuffer);
		m_UniformBuffer = nullptr;
	}
}

bool wgpu::Material::SetTexture(int binding, const Texture2D* texture)
{
	if (texture == nullptr) return false;

	m_Textures.insert_or_assign(binding, texture);
	m_UpdateBindGroup = true;

	return true;
}

bool wgpu::Material::SetSampler(int binding, const Sampler* sampler)
{
	if (sampler == nullptr) return false;

	m_Samplers.insert_or_assign(binding, sampler);
	m_UpdateBindGroup = true;

	return true;
}

bool wgpu::Material::SetFrame(int binding, WGPUTextureView frameTextureView)
{
	if (frameTextureView == nullptr) return false;

	const auto layout = m_Pipeline->GetBindGroupLayout();

	const int requiredBinding = layout->GetFrameEntryBinding();
	if (requiredBinding < 0) return false;
	if (requiredBinding != binding) return false;

	m_FrameBuffer.binding = binding;
	m_FrameBuffer.textureView = frameTextureView;
	m_UpdateBindGroup = true;

	return true;
}

WGPUBindGroup wgpu::Material::GetBindGroup()
{
	UpdateUniformBuffer();
	UpdateBindgroup();

	return m_BindGroup;
}

void wgpu::Material::UpdateUniformBuffer()
{
	if (!m_UpdateUniformBuffer) return;
	if (!m_Uniform.any.has_value()) return;

	const auto queue = m_Pipeline->GetContext()->GetQueue();
	wgpuQueueWriteBuffer
	(
		queue,
		m_UniformBuffer,
		0,
		m_Uniform.GetData(m_Uniform.any),
		m_Uniform.size
	);

	m_UpdateUniformBuffer = false;
}

void wgpu::Material::UpdateBindgroup()
{
	if (!m_UpdateBindGroup) return;

	if (m_BindGroup != nullptr)
	{
		wgpuBindGroupRelease(m_BindGroup);
		m_BindGroup = nullptr;
	}

	std::vector<WGPUBindGroupEntry> bindings{};

	auto bindGroupLayout = m_Pipeline->GetBindGroupLayout();

	if (m_Uniform.any.has_value() && bindGroupLayout->RequiresUniform())
	{
		WGPUBindGroupEntry entry{};

		entry.binding = m_Uniform.binding;
		entry.buffer = m_UniformBuffer;
		entry.offset = 0;
		entry.size = m_Uniform.size;
		bindings.push_back(entry);
	}
	else if (bindGroupLayout->RequiresUniform())
	{
		throw std::runtime_error("The uniform required has not been set!");
	}

	for (const auto& [binding, texture] : m_Textures)
	{
		WGPUBindGroupEntry entry{};

		entry.binding = binding;
		entry.textureView = texture->GetView();

		bindings.push_back(entry);
	}

	for (const auto& [binding, sampler] : m_Samplers)
	{
		WGPUBindGroupEntry entry{};

		entry.binding = binding;
		entry.sampler = sampler->GetSampler();

		bindings.push_back(entry);
	}

	bool needsFrameBuffer = bindGroupLayout->GetFrameEntryBinding() >= 0;
	if (m_FrameBuffer.textureView != nullptr && needsFrameBuffer)
	{
		WGPUBindGroupEntry entry{};

		entry.binding = m_FrameBuffer.binding;
		entry.textureView = m_FrameBuffer.textureView;
		bindings.push_back(entry);
	}
	else if (needsFrameBuffer)
	{
		throw std::runtime_error("The frame buffer required has not been set!");
	}

	WGPUBindGroupDescriptor bindGroupDesc{};
	bindGroupDesc.nextInChain = nullptr;
	bindGroupDesc.layout = bindGroupLayout->GetLayout();
	bindGroupDesc.entryCount = bindings.size();
	bindGroupDesc.entries = bindings.data();

	auto device = m_Pipeline->GetContext()->GetDevice();
	m_BindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDesc);
	
	m_UpdateBindGroup = false;
}
