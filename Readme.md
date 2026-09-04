# Lettuce

Lettuce is a rendering Vulkan-based library written in C++ 23.
The focus of Lettuce is be a library of real-time graphics oriented to design and engineering with the better
performance as possible.

## Organization

- include: c++ header files
- src: c++ source files
- docs: documentation
- samples: samples of Lettuce

## Features:

_Core_: RHI built on top of Vulkan 1.3
- Automatic Synchronization.
- Automatic Swapchain resizing.
- Transient Command Buffers.
- Descriptor Heap model (Storage Textures, Samplers and Sampled Images heaps).
- Based on Handle System.

_Foundations_: Base infrastructure and data structures using RAII for Lettuce.
- Gpu Containers.
- Gpu Unique Pointer.

_Quimera_: Renderer for 2D graphics.
- One pass library.
- Geometry is coordinates-independent.
- Based on hardware rasterizer.

_Rendering_: Passes for 3D Rendering.
- Scene View
- Debug Pass

_Utils_: Common Utils for users.
- Asset Loader for KTX2 and GLTF2.
- Frame Timer.
- Camera3D quaternion-based.

## Basic samples

### Device Creation
```cpp
DeviceDesc deviceCI = {
    .preferDedicated = true,
};
auto device = std::make_unique<Device>(deviceCI);
```

### Swapchain Creation
```cpp
SwapchainDesc swapchainDesc = {
    .clipped = true,
    .windowPtr = hwnd,
    .applicationPtr = hmodule,
};
auto swapchain = device->CreateSwapchain(swapchainDesc);
```

### Surface Creation
```cpp
SurfaceDesc surfaceCI = {
    .device = *device,
    .maxImplicitGeometries = 10000,
    .maxBrushes = 10000,
    .maxDrawCommands = 10000,
    .colorOutputFormat = device->GetRenderTargetFormat(swapchain),
};
auto surface = std::make_unique<Surface>(surfaceCI) 
```

### Load GLTF2 Model
```cpp
auto srcs = std::vector<GeometrySource>();
srcs.push_back(Lettuce::Utils::AssetLoader::LoadGtlfAsGeometry(device.get(), modelPath.string()));

SceneViewDesc sceneDesc = {
    .device = *device,
    .sources = srcs,
    .maxInstanceCount = 20,
};
scene = std::make_unique<SceneView>(sceneDesc);

sceneViewData->instanceCount = scene->GetInstanceTable().size();
```

### Load KTX2 Texture
```cpp
auto myTex = AssetLoader::LoadKtx2Texture(&device, copyCmd, path);
```

# License
MIT