#include "GraphicsContext.h"

#include <iostream>
#include <assert.h>

#include <SDL3/SDL.h>
#include <sdl3webgpu.h>
#include <glm/glm.hpp>

wgpu::GraphicsContext::GraphicsContext(SDL_Window* window)
	: m_Window{ window }
{
	if (!CreateInstance())
	{
		throw std::runtime_error("Failed to initialize WebGPU!");
	}

	if (!RequestAdapter())
	{
		throw std::runtime_error("Failed to Request Adapter!");
	}

	if (!RequestDevice())
	{
		throw std::runtime_error("Failed to request Device!");
	}

	if (!RequestQueue())
	{
		throw std::runtime_error("Failed to request Queue from device!");
	}

	if (!InitSurface())
	{
		throw std::runtime_error("Failed to initialize surface!");
	}
}

wgpu::GraphicsContext::~GraphicsContext()
{
	m_Window = nullptr;

	wgpuSurfaceRelease(m_Surface);
	m_Surface = nullptr;

	wgpuQueueRelease(m_Queue);
	m_Queue = nullptr;

	wgpuDeviceRelease(m_Device);
	m_Device = nullptr;

	wgpuAdapterRelease(m_Adapter);
	m_Adapter = nullptr;

	wgpuInstanceRelease(m_Instance);
	m_Instance = nullptr;
}

bool wgpu::GraphicsContext::CreateInstance()
{
	WGPUInstanceDescriptor desc{};
	desc.nextInChain = nullptr;

	m_Instance = wgpuCreateInstance(&desc);

	return m_Instance != nullptr;
}

bool wgpu::GraphicsContext::RequestAdapter()
{
	WGPURequestAdapterOptions options = {};
	options.nextInChain = nullptr;

	auto onRequestEnded =
		[](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userData1, void* userData2)
		{
			if (status == WGPURequestAdapterStatus_Success)
			{
				WGPUAdapter& adapterReturn = *reinterpret_cast<WGPUAdapter*>(userData1);
				adapterReturn = adapter;
			}
			else
			{
				std::cout << "Could not get WebGPU adapter: " << message.data << "\n";
			}

			bool& requestEnded = *reinterpret_cast<bool*>(userData2);
			requestEnded = true;
		};

	bool requestEnded = false;

	WGPURequestAdapterCallbackInfo info{};
	info.callback = onRequestEnded;
	info.userdata1 = &m_Adapter;
	info.userdata2 = &requestEnded;

	wgpuInstanceRequestAdapter(m_Instance, &options, info);

	assert(requestEnded);

	return m_Adapter != nullptr;
}

bool wgpu::GraphicsContext::RequestDevice()
{
	WGPUDeviceDescriptor desc{};

	desc.nextInChain = nullptr;
	desc.label = WGPUStringView("My Device");	// anything works here, that's your call
	desc.requiredFeatureCount = 0;				// I don't require any specific feature
	desc.requiredLimits = nullptr;				// I don't require any specific limit
	desc.defaultQueue.nextInChain = nullptr;
	desc.defaultQueue.label = WGPUStringView("The default queue");
	desc.deviceLostCallbackInfo.callback = nullptr;

	// TODO
	//WGPULimits requiredLimits = GetRequiredLimits(m_Adapter);
	//deviceDesc.requiredLimits = &requiredLimits;

	auto onDeviceRequestEnded =
		[](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userData1, void* userData2)
		{
			if (status == WGPURequestDeviceStatus_Success)
			{
				WGPUDevice& deviceReturn = *reinterpret_cast<WGPUDevice*>(userData1);
				deviceReturn = device;
			}
			else
			{
				std::cout << "Could not get WebGPU device: " << message.data << "\n";
			}
			bool& requestEnded = *reinterpret_cast<bool*>(userData2);
			requestEnded = true;
		};

	bool requestEnded = false;

	WGPURequestDeviceCallbackInfo info{};
	info.callback = onDeviceRequestEnded;
	info.userdata1 = &m_Device;
	info.userdata2 = &requestEnded;

	wgpuAdapterRequestDevice(m_Adapter, &desc, info);

	assert(requestEnded);

	return m_Device != nullptr;
}

bool wgpu::GraphicsContext::RequestQueue()
{
	m_Queue = wgpuDeviceGetQueue(m_Device);

	return m_Queue != nullptr;
}

bool wgpu::GraphicsContext::InitSurface()
{
	m_Surface = SDL_GetWGPUSurface(m_Instance, m_Window);

	if (m_Surface == nullptr)
	{
		return false;
	}

	WGPUSurfaceConfiguration config = {};
	config.nextInChain = nullptr;

	glm::ivec2 size{};
	SDL_GetWindowSize(m_Window, &size.x, &size.y);

	config.width = size.x;
	config.height = size.y;

	WGPUSurfaceCapabilities capabilities{};
	capabilities.nextInChain = nullptr;

	const auto result = wgpuSurfaceGetCapabilities(m_Surface, m_Adapter, &capabilities);
	if (result != WGPUStatus_Success)
	{
		std::cerr << "Failed to get surface capabilities!\n";
		return false;
	}

	// forcing a format can solve incorrect gamma but can also result in preformance issues
	// solution => adjust your fragment shader
	//m_SurfaceFormat = WGPUTextureFormat_RGBA8Unorm;
	m_SurfaceFormat = capabilities.formats[0];
	config.format = m_SurfaceFormat;

	config.viewFormatCount = 0;
	config.viewFormats = nullptr;

	config.usage = WGPUTextureUsage_RenderAttachment;
	config.device = m_Device;

	config.presentMode = WGPUPresentMode_Fifo;
	config.alphaMode = WGPUCompositeAlphaMode_Auto;

	wgpuSurfaceConfigure(m_Surface, &config);

	return true;
}
