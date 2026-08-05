#include "RenderQueue.h"

#include <stdexcept>
#include "Material.h"

const size_t wgpu::RenderQueue::m_InitialBatchSize{ 512 };

void wgpu::RenderQueue::PushTriangle(Material& mat, const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2)
{
	auto [itr, inserted] = m_Batches.try_emplace(&mat, std::vector<Vertex3D>());

	if (inserted)
	{
		itr->second.reserve(m_InitialBatchSize);
	}

	itr->second.push_back(v0);
	itr->second.push_back(v1);
	itr->second.push_back(v2);
}

void wgpu::RenderQueue::Flush()
{
	m_Batches.clear();
}

void wgpu::RenderQueue::Render(const GraphicsContext& context, WGPUBuffer vertexBuffer, WGPURenderPassEncoder renderPass) const
{
	if (m_Batches.empty()) return;

	// write to the vertex buffer
	std::vector<Vertex3D> allVertices{};
	allVertices.reserve(GetBufferSize() / sizeof(Vertex3D));

	for (const auto& [mat, batch] : m_Batches)
	{
		allVertices.insert(allVertices.end(), batch.begin(), batch.end());
	}

	wgpuQueueWriteBuffer(context.GetQueue(), vertexBuffer, 0, allVertices.data(), allVertices.size() * sizeof(Vertex3D));

	// render vertices...
	uint64_t offset{ 0 };

	for (auto& [mat, batch] : m_Batches)
	{
		wgpuRenderPassEncoderSetPipeline(renderPass, mat->GetPipeline()->GetPipeline());

		if (mat->GetPipeline()->GetBindGroupLayout() != nullptr && mat->GetBindGroup() != nullptr)
		{
			wgpuRenderPassEncoderSetBindGroup(renderPass, 0, mat->GetBindGroup(), 0, nullptr);
		}

		wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, offset, batch.size() * sizeof(Vertex3D));
		wgpuRenderPassEncoderDraw(renderPass, static_cast<uint32_t>(batch.size()), 1, 0, 0);
		
		offset += batch.size() * sizeof(Vertex3D);
	}
}

void wgpu::RenderQueue::SetCamera(const CameraData& camera)
{
	for (auto [material, _] : m_Batches)
	{
		const int binding = material->GetUniformBinding<CameraData>();
		
		if (binding < 0) continue;

		material->SetUniform(binding, camera);
	}
}

size_t wgpu::RenderQueue::GetBufferSize() const
{
	size_t bufferSize{ 0 };

	for (const auto& [mat, batch] : m_Batches)
	{
		bufferSize += batch.size();
	}

	return bufferSize * sizeof(Vertex3D);
}
