#ifndef RENDERQUEUE
#define RENDERQUEUE

#include <unordered_map>
#include <vector>

#include <DataTypes.h>
#include "GraphicsContext.h"

namespace wgpu
{
	class Material;
	class RenderQueue final
	{
	public:
		RenderQueue() = default;
		~RenderQueue() = default;

		RenderQueue(const RenderQueue&) = delete;
		RenderQueue& operator=(const RenderQueue&) = delete;
		RenderQueue(RenderQueue&&) = delete;
		RenderQueue& operator=(RenderQueue&&) = delete;

		void PushTriangle(Material& mat, const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2);
		void Flush();

		void Render(const GraphicsContext& context, WGPUBuffer vertexBuffer, WGPURenderPassEncoder renderPass) const;
		void Render(const GraphicsContext& context, WGPUBuffer vertexBuffer, WGPURenderPassEncoder opaquePass, WGPURenderPassEncoder transparentPass) const;
		void SetCamera(const CameraData& camera);

		size_t GetBufferSize() const;

	private:
		// can also be done with two vetors:
		// vector a : stores al vertices
		// vector b : stores batches => {material*, first vertex idx, num vertices}
		std::unordered_map<Material*, std::vector<Vertex3D>> m_Batches{};

		static const size_t m_InitialBatchSize;
	};
}

#endif
