#include "RenderQueue.h"

#include <stdexcept>
#include <algorithm>

#include <Material.h>
#include <Renderer2D.h>

#include "GraphicsContext.h"

const size_t wgpu::RenderQueue::m_InitialBatchSize{ 512 };

void wgpu::RenderQueue::SubmitMesh(Material* material, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices)
{
	// TODO : Figure out what happens when inputting drifferent kinds of geometry!

	if (material == nullptr)
	{
		throw std::runtime_error("Unable to submit a mesh, since given material is nullptr!");
	}

	Batch newBatch{};
	newBatch.material = material;
	newBatch.firstVertex = static_cast<uint32_t>(m_Verices.size());
	newBatch.firstIndex = static_cast<uint32_t>(m_Indices.size());
	newBatch.indexCount = static_cast<uint32_t>(indices.size());

	m_Batches.emplace_back(newBatch);

	m_Verices.insert(m_Verices.end(), vertices.begin(), vertices.end());
	m_Indices.insert(m_Indices.end(), indices.begin(), indices.end());
}

void wgpu::RenderQueue::Flush()
{
	m_Batches.clear();
	m_Verices.clear();
	m_Indices.clear();
}

void wgpu::RenderQueue::Render(const Renderer2D& renderer, WGPUBuffer vertexBuffer, WGPUBuffer indexBuffer, WGPURenderPassEncoder renderPass)
{
	const auto context = renderer.GetContext();

	wgpuQueueWriteBuffer(context->GetQueue(), vertexBuffer, 0, m_Verices.data(), GetVertexBufferSize());
	wgpuQueueWriteBuffer(context->GetQueue(), indexBuffer, 0, m_Indices.data(), GetIndexBufferSize());

	std::ranges::sort
	(
		m_Batches,
		[](const Batch& a, const Batch& b)
		{
			return a.material < b.material;
		}
	);

	Material* lastMaterial{ nullptr };

	wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0, wgpuBufferGetSize(vertexBuffer));
	wgpuRenderPassEncoderSetIndexBuffer(renderPass, indexBuffer, WGPUIndexFormat_Uint32, 0, wgpuBufferGetSize(indexBuffer));

	for (auto& batch : m_Batches)
	{
		if (lastMaterial != batch.material)
		{
			lastMaterial = batch.material;

			// set GPU state
			wgpuRenderPassEncoderSetPipeline(renderPass, batch.material->GetPipeline()->GetPipeline());

			if (
				batch.material->GetPipeline()->GetBindGroupLayout() != nullptr && 
				batch.material->GetBindGroup() != nullptr
			)
			{
				wgpuRenderPassEncoderSetBindGroup(renderPass, 0, batch.material->GetBindGroup(), 0, nullptr);
			}
		}

		wgpuRenderPassEncoderDrawIndexed(renderPass, batch.indexCount, 1, batch.firstIndex, batch.firstVertex, 0);
	}
}

size_t wgpu::RenderQueue::GetVertexBufferSize() const
{
	return m_Verices.size() * sizeof(Vertex3D);
}

size_t wgpu::RenderQueue::GetIndexBufferSize() const
{
	return m_Indices.size() * sizeof(uint32_t);
}
