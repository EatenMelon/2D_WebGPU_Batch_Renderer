#include "Material.h"

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

    m_UniformBuffer = wgpuDeviceCreateBuffer(m_Pipeline->GetGraphicsContext()->GetDevice(), &bufferDesc);
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

	std::vector<Uniform> uniforms{};
	GetSortedUniforms(uniforms);

	const auto queue = m_Pipeline->GetGraphicsContext()->GetQueue();
	uint64_t offset{ 0 };

	for (auto& uniform : uniforms)
	{
		wgpuQueueWriteBuffer
		(
			queue,
			m_UniformBuffer,
			offset,
			uniform.GetData(uniform.value),
			uniform.size
		);

		offset += uniform.size;
	};

	m_UpdateUniformBuffer = false;
}

void wgpu::Material::UpdateBindgroup()
{
	if (!m_UpdateBindGroup) return;

	std::vector<Uniform> uniforms{};
	GetSortedUniforms(uniforms);

	std::vector<WGPUBindGroupEntry> bindings{};
	uint64_t offset{ 0 };

	for (auto& uniform : uniforms)
	{
		WGPUBindGroupEntry entry{};

		entry.binding = uniform.binding;
		entry.buffer = m_UniformBuffer;
		entry.offset = offset;
		entry.size = uniform.size;
		bindings.push_back(entry);

		offset += uniform.size;
	};

	WGPUBindGroupDescriptor bindGroupDesc{};
	bindGroupDesc.nextInChain = nullptr;
	bindGroupDesc.layout = m_Pipeline->GetBindGroupLayout()->GetLayout();
	bindGroupDesc.entryCount = bindings.size();
	bindGroupDesc.entries = bindings.data();

	auto device = m_Pipeline->GetGraphicsContext()->GetDevice();
	m_BindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDesc);
	
	m_UpdateBindGroup = false;
}

void wgpu::Material::GetSortedUniforms(std::vector<Uniform>& out) const
{
	for (const auto& [binding, uniform] : m_Uniforms)
	{
		out.push_back(uniform);
	}

	std::sort
	(
		out.begin(),
		out.end(),
		[](const Uniform& a, const Uniform& b)
		{
			return a.binding < b.binding;
		}
	);
}
