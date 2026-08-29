#include <Renderer2D.h>

#include <Material.h>
#include <BindGroupLayout.h>

#include "GraphicsContext.h"
#include "RenderQueue.h"
#include "BuiltinResources.h"

#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_sdl3.h>

wgpu::Renderer2D::Renderer2D(SDL_Window* window)
	: m_Camera{ std::make_shared<wgpu::Camera2D>() }
	, m_RenderQueue{ std::make_unique<RenderQueue>() }
	, m_Context{ std::make_unique<GraphicsContext>(window) }
{
	InitDepthBuffer();
	CreateVertexBuffer(100 * sizeof(Vertex3D));
	CreateIndexBuffer(50 * sizeof(uint32_t));

	m_Camera->SetAspectRatio(m_Context->GetAspectRatio());

	m_BuiltinResources = std::make_unique<BuiltinResources>(*this);

	InitPostProcessingData();
	ImGuiInit();
}

wgpu::Renderer2D::~Renderer2D() noexcept
{
	ImGuiQuit();
	wgpuBufferRelease(m_Vertex.buffer);
	m_Vertex.buffer = nullptr;

	wgpuBufferRelease(m_Index.buffer);
	m_Index.buffer = nullptr;

	ReleaseDepthBuffer();

	DestroyPostProcessingData();
}

void wgpu::Renderer2D::BatchMesh(Material* material, const Mesh3D& mesh) const
{
	m_RenderQueue->SubmitMesh(material, mesh);
}

void wgpu::Renderer2D::SubmitPostProcessingEffect(Material* material)
{
	if (material == nullptr) return;

	m_PostEffects.push_back(material);
}

void wgpu::Renderer2D::BeginFrame()
{
	m_Context->UpdateWindowFlags();

	m_RenderQueue->Flush();
	m_PostEffects.clear();

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

	m_PostEffects.clear();
}

void wgpu::Renderer2D::GuiBeginFrame()
{
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void wgpu::Renderer2D::GuiEndFrame()
{
	ImGui::EndFrame();
}

void wgpu::Renderer2D::Render() const
{
	if (m_Context->IsWindowMinimized()) return;
	
	// get the next target texture view
	auto [surfaceTexture, targetView] = GetNextSurfaceViewData();

	WGPUCommandEncoderDescriptor encoderDesc{};
	encoderDesc.nextInChain = nullptr;
	encoderDesc.label = WGPUStringView("Render objects", 15);
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_Context->GetDevice(), &encoderDesc);

	WGPUTextureView ping = m_PPData.pingView;
	WGPUTextureView pong = m_PPData.pongView;

	// render objects
	if (!m_PostEffects.empty())
	{
		RenderObjects(ping, encoder);
	}
	else
	{
		RenderObjects(targetView, encoder);
	}

	for (size_t idx = 0; idx < m_PostEffects.size(); ++idx)
	{
		auto effect = m_PostEffects[idx];
		const auto pipeline = effect->GetPipeline();

		if (pipeline->GetPipelineType() != Pipeline::Type::PostProcessing)
		{
			throw std::runtime_error("Non post processing material is being used for post processing effect!");
		}

		const auto bindGroup = pipeline->GetBindGroupLayout();
		const int binding = bindGroup->GetFrameEntryBinding();

		if (binding < 0)
		{
			throw std::runtime_error("Post processing effect doesn't have a frame binding!");
		}

		effect->SetFrame(binding, ping);

		if (idx >= m_PostEffects.size() - 1)
		{
			RenderPostEffect(effect, targetView, encoder);
		}
		else
		{
			RenderPostEffect(effect, pong, encoder);
			std::swap(ping, pong);
		}
	}

	RenderGui(targetView, encoder);

	// finish encoding
	WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, nullptr);
	wgpuCommandEncoderRelease(encoder);

	wgpuQueueSubmit(m_Context->GetQueue(), 1, &commandBuffer);
	wgpuCommandBufferRelease(commandBuffer);

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

void wgpu::Renderer2D::Resize()
{
	m_Context->DestroySurface();
	ReleaseDepthBuffer();

	m_Context->InitSurface();
	InitDepthBuffer();

	DestroyPostProcessingData();
	InitPostProcessingData();

	m_Camera->SetAspectRatio(m_Context->GetAspectRatio());
}

void wgpu::Renderer2D::SetCamera(const std::shared_ptr<Camera2D>& camera)
{
	m_Camera = camera;
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

void wgpu::Renderer2D::ImGuiInit()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO();

	ImGui_ImplSDL3_InitForOther(m_Context->GetWindow());

	ImGui_ImplWGPU_InitInfo info{};
	info.Device = m_Context->GetDevice();
	info.NumFramesInFlight = 3;
	info.DepthStencilFormat = wgpuTextureGetFormat(m_DepthTexture);
	info.RenderTargetFormat = m_Context->GetSurfaceFormat();

	ImGui_ImplWGPU_Init(&info);
}

void wgpu::Renderer2D::ImGuiQuit()
{
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplWGPU_Shutdown();
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

void wgpu::Renderer2D::InitPostProcessingData()
{
	const auto windowSize = m_Context->GetWindowSize();

	WGPUTextureDescriptor texDesc{};
	texDesc.nextInChain = nullptr;
	texDesc.format = m_Context->GetSurfaceFormat();
	texDesc.dimension = WGPUTextureDimension_2D;
	texDesc.sampleCount = 1;
	texDesc.mipLevelCount = 1;
	texDesc.size.width = static_cast<uint32_t>(windowSize.x);
	texDesc.size.height = static_cast<uint32_t>(windowSize.y);
	texDesc.size.depthOrArrayLayers = 1;
	texDesc.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;

	m_PPData.pongTexture = wgpuDeviceCreateTexture(GetContext()->GetDevice(), &texDesc);
	m_PPData.pingTexture = wgpuDeviceCreateTexture(GetContext()->GetDevice(), &texDesc);

	WGPUTextureViewDescriptor viewDesc{};
	viewDesc.nextInChain = nullptr;
	viewDesc.format = m_Context->GetSurfaceFormat();
	viewDesc.dimension = WGPUTextureViewDimension_2D;
	viewDesc.baseMipLevel = 0;
	viewDesc.mipLevelCount = 1;
	viewDesc.baseArrayLayer = 0;
	viewDesc.arrayLayerCount = 1;
	viewDesc.aspect = WGPUTextureAspect_All;
	viewDesc.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;

	m_PPData.pingView = wgpuTextureCreateView(m_PPData.pingTexture, &viewDesc);
	m_PPData.pongView = wgpuTextureCreateView(m_PPData.pongTexture, &viewDesc);
}

void wgpu::Renderer2D::DestroyPostProcessingData()
{
	wgpuTextureRelease(m_PPData.pingTexture);
	wgpuTextureRelease(m_PPData.pongTexture);
	wgpuTextureViewRelease(m_PPData.pingView);
	wgpuTextureViewRelease(m_PPData.pongView);
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

std::pair<WGPUSurfaceTexture, WGPUTextureView> wgpu::Renderer2D::GetNextSurfaceViewData() const
{
	WGPUTextureView view{ nullptr };
	WGPUSurfaceTexture surfaceTexture{};

	wgpuSurfaceGetCurrentTexture(m_Context->GetSurface(), &surfaceTexture);

	if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal)
	{
		return { surfaceTexture, view };
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
	viewDescriptor.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;

	view = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

	return { surfaceTexture, view };
}

void wgpu::Renderer2D::RenderObjects(WGPUTextureView targetView, WGPUCommandEncoder encoder) const
{
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

	// end renderpass
	wgpuRenderPassEncoderEnd(renderPass);
	wgpuRenderPassEncoderRelease(renderPass);
}

void wgpu::Renderer2D::RenderPostEffect(Material* effect, WGPUTextureView targetView, WGPUCommandEncoder encoder) const
{
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
	WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

	auto pipeline = effect->GetPipeline();
	wgpuRenderPassEncoderSetPipeline(renderPass, pipeline->GetPipeline());

	if (pipeline->GetBindGroupLayout() != nullptr && effect->GetBindGroup() != nullptr)
	{
		wgpuRenderPassEncoderSetBindGroup(renderPass, 0, effect->GetBindGroup(), 0, nullptr);
	}

	wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);

	// end renderpass
	wgpuRenderPassEncoderEnd(renderPass);
	wgpuRenderPassEncoderRelease(renderPass);
}

void wgpu::Renderer2D::RenderGui(WGPUTextureView targetView, WGPUCommandEncoder encoder) const
{
	if (ImGui::GetFrameCount() <= 0) return;

	ImGui::Render();

	// describe render pass
	WGPURenderPassDescriptor renderPassDesc{};
	renderPassDesc.nextInChain = nullptr;
	renderPassDesc.depthStencilAttachment = nullptr;
	renderPassDesc.timestampWrites = nullptr;

	WGPURenderPassColorAttachment renderPassColorAttachment{};
	renderPassColorAttachment.view = targetView;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = WGPULoadOp_Load;
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

	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);

	// end renderpass
	wgpuRenderPassEncoderEnd(renderPass);
	wgpuRenderPassEncoderRelease(renderPass);
}

