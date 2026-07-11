#include "RenderQueue.h"

#include <stdexcept>

void wgpu::RenderQueue::PushTriangle(const Material& mat, const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	if (m_Batches.contains(&mat))
	{
		auto itr = m_Batches.find(&mat);

		itr->second.push_back(v0);
		itr->second.push_back(v1);
		itr->second.push_back(v2);
		return;
	}

	auto [itr, inserted] = m_Batches.emplace(&mat, std::vector<Vertex>());

	if (!inserted)
	{
		throw std::runtime_error("Failed to create new batch!");
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
	// write to the vertex buffer
	std::vector<Vertex> allVertices{};

	for (const auto& [mat, batch] : m_Batches)
	{
		allVertices.insert(allVertices.end(), batch.begin(), batch.end());
	}

	wgpuQueueWriteBuffer(context.GetQueue(), vertexBuffer, 0, allVertices.data(), allVertices.size() * sizeof(Vertex));

	// render vertices...
	uint64_t offset{ 0 };

	for (const auto& [mat, batch] : m_Batches)
	{
		wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, offset, batch.size() * sizeof(Vertex));
		wgpuRenderPassEncoderDraw(renderPass, static_cast<uint32_t>(batch.size()), 1, 0, 0);
		
		offset += batch.size() * sizeof(Vertex);
	}
}

size_t wgpu::RenderQueue::GetBufferSize() const
{
	size_t bufferSize{ 0 };

	for (const auto& [mat, batch] : m_Batches)
	{
		bufferSize += batch.size();
	}

	return bufferSize * sizeof(Vertex);
}
