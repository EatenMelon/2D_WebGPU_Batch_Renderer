#ifndef RENDERQUEUE
#define RENDERQUEUE

#include <vector>

#include <DataTypes.h>
#include <wgpu.h>

namespace wgpu
{
	class Material;
	class Renderer2D;
	class RenderQueue final
	{
	public:
		RenderQueue() = default;
		~RenderQueue() = default;

		RenderQueue(const RenderQueue&) = delete;
		RenderQueue& operator=(const RenderQueue&) = delete;
		RenderQueue(RenderQueue&&) = delete;
		RenderQueue& operator=(RenderQueue&&) = delete;

		void SubmitMesh(Material* material, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);
		void Flush();

		void Render(const Renderer2D& renderer, WGPUBuffer vertexBuffer, WGPUBuffer indexBuffer, WGPURenderPassEncoder renderPass);

		size_t GetVertexBufferSize() const;
		size_t GetIndexBufferSize() const;

	private:
		// acts as a cursor for the stored index and vertex arrays
		struct Batch
		{
			Material* material{ nullptr };
			uint32_t firstIndex{ 0 };
			uint32_t indexCount{ 0 };
			uint32_t firstVertex{ 0 };
		};

		std::vector<Batch> m_Batches{};
		std::vector<Vertex3D> m_Verices{};
		std::vector<uint32_t> m_Indices{};

		static const size_t m_InitialBatchSize;
	};
}

#endif
