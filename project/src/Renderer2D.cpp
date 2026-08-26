#include <Renderer2D.h>

#include "GraphicsContext.h"
#include "RenderQueue.h"
#include "BuiltinResources.h"

wgpu::Renderer2D::Renderer2D(SDL_Window* window)
	: m_Camera{ std::make_shared<wgpu::Camera2D>() }
	, m_Context{ std::make_unique<GraphicsContext>(window) }
	, m_RenderQueue{ std::make_unique<RenderQueue>() }
{
	InitDepthBuffer();
	CreateVertexBuffer(100 * sizeof(Vertex3D));
	CreateIndexBuffer(100 * sizeof(uint32_t));

	m_Camera->SetAspectRatio(m_Context->GetAspectRatio());

	m_BuiltinResources = std::make_unique<BuiltinResources>(*this);

}

wgpu::Renderer2D::~Renderer2D() noexcept
{
	wgpuBufferRelease(m_Vertex.buffer);
	m_Vertex.buffer = nullptr;
}

void wgpu::Renderer2D::BeginFrame()
{
	m_Context->UpdateWindowFlags();

	m_RenderQueue->Flush();

	if (m_Context->IsWindowMinimized()) return;

	// clear the vertex buffer
	WGPUCommandEncoderDescriptor desc{};
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_Context->GetDevice(), &desc);

	wgpuCommandEncoderClearBuffer(encoder, m_Vertex.buffer, 0, WGPU_WHOLE_SIZE);

	WGPUCommandBufferDescriptor cmdDesc{};
	WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(encoder, &cmdDesc);

	wgpuQueueSubmit(m_Context->GetQueue(), 1, &cmd);

	wgpuCommandBufferRelease(cmd);
	wgpuCommandEncoderRelease(encoder);
}

void wgpu::Renderer2D::BatchMesh(Material* material, const Mesh3D& mesh) const
{
	m_RenderQueue->SubmitMesh(material, mesh);
}

void wgpu::Renderer2D::EndFrame()
{
	if (m_Context->IsWindowMinimized()) return;

	const size_t vertexBufferSize = m_RenderQueue->GetVertexBufferSize();
	if (m_Vertex.capacity < vertexBufferSize)
	{
		CreateVertexBuffer(vertexBufferSize * 2);
	}

	const size_t indexBufferSize = m_RenderQueue->GetIndexBufferSize();
	if (m_Index.capacity < indexBufferSize)
	{
		CreateIndexBuffer(indexBufferSize * 2);
	}
}

void wgpu::Renderer2D::Render() const
{
	if (m_Context->IsWindowMinimized()) return;

	// get the next target texture view
	auto [surfaceTexture, targetView] = GetNextSurfaceViewData();
	if (!targetView) return;

	// render objects
	{
		WGPUCommandEncoderDescriptor encoderDesc{};
		encoderDesc.nextInChain = nullptr;
		encoderDesc.label = WGPUStringView("Render objects");
		WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_Context->GetDevice(), &encoderDesc);

		// describe render pass
		WGPURenderPassDescriptor renderPassDesc{};
		renderPassDesc.nextInChain = nullptr;
		renderPassDesc.depthStencilAttachment = nullptr;
		renderPassDesc.timestampWrites = nullptr;

		WGPURenderPassColorAttachment renderPassColorAttachment{};
		renderPassColorAttachment.view = targetView;
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
		renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
		renderPassColorAttachment.clearValue = { m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a };
		renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;

		// Setup depth/stencil
		WGPURenderPassDepthStencilAttachment depthStencilAttachment{};

		depthStencilAttachment.view = m_DepthTextureView;
		depthStencilAttachment.depthClearValue = 1.f;
		depthStencilAttachment.depthLoadOp = WGPULoadOp_Clear;
		depthStencilAttachment.depthStoreOp = WGPUStoreOp_Store;
		depthStencilAttachment.depthReadOnly = false;

		// Stencil setup, mandatory but unused
		depthStencilAttachment.stencilClearValue = 0;
		depthStencilAttachment.stencilLoadOp = WGPULoadOp_Undefined;
		depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Undefined;
		depthStencilAttachment.stencilReadOnly = true;

		renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

		WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

		// use render pass
		// -> Render objects here!
		m_RenderQueue->Render(*this, m_Vertex.buffer, m_Index.buffer, renderPass);

		// end renderpasses
		wgpuRenderPassEncoderEnd(renderPass);
		wgpuRenderPassEncoderRelease(renderPass);

		// finish encoding
		WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, nullptr);
		wgpuCommandEncoderRelease(encoder);

		wgpuQueueSubmit(m_Context->GetQueue(), 1, &commandBuffer);
		wgpuCommandBufferRelease(commandBuffer);
	}
	
	// present surface onto window
	wgpuSurfacePresent(m_Context->GetSurface());

	// cleanup
	wgpuTextureRelease(surfaceTexture.texture);
	wgpuTextureViewRelease(targetView);
}

void wgpu::Renderer2D::SetClearColor(const ColorF& color)
{
	m_ClearColor = color;
}

void wgpu::Renderer2D::SetCamera(const std::shared_ptr<Camera2D>& camera)
{
	m_Camera = camera;
	m_Camera->SetAspectRatio(m_Context->GetAspectRatio());
}

void wgpu::Renderer2D::Resize()
{
	m_Context->DestroySurface();
	ReleaseDepthBuffer();

	m_Context->InitSurface();
	InitDepthBuffer();

	m_Camera->SetAspectRatio(m_Context->GetAspectRatio());
}

std::shared_ptr<wgpu::Camera2D> wgpu::Renderer2D::GetCamera() const
{
	return m_Camera;
}

wgpu::Material* wgpu::Renderer2D::GetSolidColorMaterial() const
{
	return m_BuiltinResources->GetSolidColorMaterial();
}

std::pair<WGPUSurfaceTexture, WGPUTextureView> wgpu::Renderer2D::GetNextSurfaceViewData() const
{
	WGPUSurfaceTexture surfaceTexture{};
	wgpuSurfaceGetCurrentTexture(m_Context->GetSurface(), &surfaceTexture);

	if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
	{
		return { surfaceTexture, nullptr };
	}

	WGPUTextureViewDescriptor viewDescriptor;
	viewDescriptor.nextInChain = nullptr;
	viewDescriptor.label = WGPUStringView("Surface texture view");
	viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
	viewDescriptor.dimension = WGPUTextureViewDimension_2D;
	viewDescriptor.baseMipLevel = 0;
	viewDescriptor.mipLevelCount = 1;
	viewDescriptor.baseArrayLayer = 0;
	viewDescriptor.arrayLayerCount = 1;
	viewDescriptor.aspect = WGPUTextureAspect_All;
	viewDescriptor.usage = WGPUTextureUsage_RenderAttachment;

	WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

	return { surfaceTexture, targetView };
}

void wgpu::Renderer2D::InitDepthBuffer()
{
	WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth24Plus;

	// Create the depth texture
	WGPUTextureDescriptor textureDesc{};
	textureDesc.dimension = WGPUTextureDimension_2D;
	textureDesc.format = depthTextureFormat;
	textureDesc.mipLevelCount = 1;
	textureDesc.sampleCount = 1;

	glm::u32vec2 size = m_Context->GetWindowSize();
	textureDesc.size = { size.x, size.y, 1 };

	textureDesc.usage = WGPUTextureUsage_RenderAttachment;
	textureDesc.viewFormatCount = 1;
	textureDesc.viewFormats = &depthTextureFormat;
	m_DepthTexture = wgpuDeviceCreateTexture(m_Context->GetDevice(), &textureDesc);

	WGPUTextureViewDescriptor viewDesc{};
	viewDesc.aspect = WGPUTextureAspect_DepthOnly;
	viewDesc.baseArrayLayer = 0;
	viewDesc.arrayLayerCount = 1;
	viewDesc.baseMipLevel = 0;
	viewDesc.mipLevelCount = 1;
	viewDesc.dimension = WGPUTextureViewDimension_2D;
	viewDesc.format = depthTextureFormat;
	m_DepthTextureView = wgpuTextureCreateView(m_DepthTexture, &viewDesc);
}

void wgpu::Renderer2D::ReleaseDepthBuffer()
{
	wgpuTextureViewRelease(m_DepthTextureView);
	wgpuTextureDestroy(m_DepthTexture);
	wgpuTextureRelease(m_DepthTexture);
}

void wgpu::Renderer2D::CreateVertexBuffer(size_t capacity)
{
	WGPUBufferDescriptor desc{};
	desc.size = capacity;
	desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
	desc.mappedAtCreation = false;

	if (m_Vertex.buffer != nullptr)
	{
		wgpuBufferRelease(m_Vertex.buffer);
	}

	m_Vertex.buffer = wgpuDeviceCreateBuffer(m_Context->GetDevice(), &desc);
	m_Vertex.capacity = capacity;
}

void wgpu::Renderer2D::CreateIndexBuffer(size_t capacity)
{
	WGPUBufferDescriptor desc{};
	desc.size = capacity;
	desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
	desc.mappedAtCreation = false;

	if (m_Index.buffer != nullptr)
	{
		wgpuBufferRelease(m_Index.buffer);
	}

	m_Index.buffer = wgpuDeviceCreateBuffer(m_Context->GetDevice(), &desc);
	m_Index.capacity = capacity;
}
