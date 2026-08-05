#include <Renderer2D.h>

#include "GraphicsContext.h"
#include "RenderQueue.h"
#include "BuiltinResources.h"

#include <iostream>

wgpu::Renderer2D::Renderer2D(SDL_Window* window)
	: m_Camera{ std::make_shared<wgpu::Camera2D>() }
	, m_Context{ std::make_unique<GraphicsContext>(window) }
	, m_RenderQueue{ std::make_unique<RenderQueue>() }
{
	InitDepthBuffer();
	CreateVertexBuffer(100 * sizeof(Vertex3D));

	m_Camera->SetAspectRatio(m_Context->GetAspectRatio());

	m_BuiltinResources = std::make_unique<BuiltinResources>(*this);
}

wgpu::Renderer2D::~Renderer2D() noexcept
{
	wgpuBufferRelease(m_VertexBuffer.buffer);
	m_VertexBuffer.buffer = nullptr;
}

void wgpu::Renderer2D::BeginFrame()
{
	m_RenderQueue->Flush();

	// clear the vertex buffer
	WGPUCommandEncoderDescriptor desc{};
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_Context->GetDevice(), &desc);

	wgpuCommandEncoderClearBuffer(encoder, m_VertexBuffer.buffer, 0, WGPU_WHOLE_SIZE);

	WGPUCommandBufferDescriptor cmdDesc{};
	WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(encoder, &cmdDesc);

	wgpuQueueSubmit(m_Context->GetQueue(), 1, &cmd);

	wgpuCommandBufferRelease(cmd);
	wgpuCommandEncoderRelease(encoder);
}

void wgpu::Renderer2D::SubmitTriangle(Material& mat, const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2)
{
	m_RenderQueue->PushTriangle(mat, v0, v1, v2);
}

void wgpu::Renderer2D::SubmitQuad(Material& mat, const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2, const Vertex3D& v3)
{
	m_RenderQueue->PushTriangle(mat, v0, v1, v2);
	m_RenderQueue->PushTriangle(mat, v2, v3, v0);

}

void wgpu::Renderer2D::EndFrame()
{
	size_t bufferSize = m_RenderQueue->GetBufferSize();

	if (m_VertexBuffer.capacity < bufferSize)
	{
		CreateVertexBuffer(bufferSize * 2);
	}

	m_RenderQueue->SetCamera(m_Camera->GetCameraData());
}

void wgpu::Renderer2D::Render() const
{
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
		depthStencilAttachment.stencilLoadOp = WGPULoadOp_Clear;
		depthStencilAttachment.stencilStoreOp = WGPUStoreOp_Store;
		depthStencilAttachment.stencilReadOnly = true;

		renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

		// begin render pass
		WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

		// use render pass
		// -> Render objects here!
		m_RenderQueue->Render(*m_Context.get(), m_VertexBuffer.buffer, renderPass);

		// end renderpass
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

void wgpu::Renderer2D::CreateVertexBuffer(size_t capacity)
{
	WGPUBufferDescriptor desc{};
	desc.size = capacity;
	desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
	desc.mappedAtCreation = false;

	if (m_VertexBuffer.buffer != nullptr)
	{
		wgpuBufferRelease(m_VertexBuffer.buffer);
	}

	m_VertexBuffer.buffer = wgpuDeviceCreateBuffer(m_Context->GetDevice(), &desc);
	m_VertexBuffer.capacity = capacity;
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
