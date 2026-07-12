#include "Renderer2D.h"

#include <iostream>

bool wgpu::Renderer2D::Init(SDL_Window* window)
{
	try
	{
		m_Context = std::make_unique<GraphicsContext>(window);
		m_RenderQueue = std::make_unique<RenderQueue>();
		CreateVertexBuffer(100 * sizeof(Vertex));
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Failed to initialize 2D renderer:\t" << ex.what() << "\n";
		return false;
	}

	return true;
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

void wgpu::Renderer2D::EndFrame()
{
	size_t bufferSize = m_RenderQueue->GetBufferSize();

	if (m_VertexBuffer.capacity < bufferSize)
	{
		CreateVertexBuffer(bufferSize * 2);
	}

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

		WGPURenderPassColorAttachment renderPassColorAttachment{};
		renderPassColorAttachment.view = targetView;
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
		renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
		renderPassColorAttachment.clearValue = m_ClearColor;

		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;

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

void wgpu::Renderer2D::Quit()
{
	wgpuBufferRelease(m_VertexBuffer.buffer);
	m_VertexBuffer.buffer = nullptr;
	m_VertexBuffer.capacity = 0;

	m_Context.reset();
}

void wgpu::Renderer2D::SetClearColor(float r, float g, float b, float a)
{
	m_ClearColor = WGPUColor(r, g, b, a);
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

	m_VertexBuffer.buffer = wgpuDeviceCreateBuffer(m_Context->GetDevice(), &desc);
	m_VertexBuffer.capacity = capacity;
}
