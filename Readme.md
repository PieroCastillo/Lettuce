# Lettuce

Lettuce is a rendering Vulkan-based library.
The focus of Lettuce is be a library of real-time graphics oriented to design and engineering with the better
performance as possible.

### Docs

Tools:

- Doxygen + Doxygen Awesome, for API reference
- Docsify for Documentation

## Organization

- include: c++ header files
- src: c++ source files
- docs: documentation
- samples: samples of Lettuce

## Components

### Lettuce Core

This is the core lib of Lettuce, the main function is provide access to Vulkan and manage Vulkan objects.
|Component name |Description| Notes | Status |
|:----------------------|:--------------|:------|:-------|
|_Core:_ | | | |
|Instance || | 🟢|
|GPU || | 🟢 |
|Swapchain | | | 🟡|
|Descriptors || | 🟢 |
|Pipeline Layout || | 🟢 |
|Graphics Pipeline || | 🟢 |
|Compute Pipeline || | 🟢 |
|Shader|| | 🟢 |
|Samplers || | 🟢 |
|Dynamic states || | 🟢|
|_Memory Allocation System:_ | | | |
|BufferResource | | | 🟢|
|ImageResource | | | 🟢|
|ImageViewResource | | | 🟢|
|ResourcePool | | | 🟢|
|TranferManager | | | 🟢|

- Extensions & features:
  |Name |Extension/Feature Name |Status |
  |:----------------------|:--------------------|:--------|
  |_Required:_ | | |
  |Indirect Drawing || | 🟡 |
  |Timeline Semaphores |_(1.2 feature)_| 🟢|
  |Descriptor Indexing |_(1.2 feature)_| 🟢|
  |Draw Indirect Count| _(1.2 feature)_|🚧|
  |Synchronization2 |_(1.3 feature)_|🟢 |
  |HLSL support| _Not added_| 🚧 |
  |SLang support | _Not added_| 🚧 |
  |Dynamic Rendering |_(1.2 feature)_| 🚧|
  |Dynamic Rendering Local Read |_(1.4 feature)_| 🚧|
  |Buffer Device Address |_(1.2 feature)_|🚧 |
  |Shader Object |_(VK_EXT_shader_object)_| 🟡|
  |Device Generated Commands |_(VK_EXT_device_generated_commands)_| 🟡|
  | _Optional:_| | |
  |Memory Budget | _(VK_EXT_memory_budget)_| 🚧|
  |Conservative Rasterization |_(VK_EXT_conservative_rasterization)_|🚧 |
  |Mesh shading |_(VK_EXT_mesh_shader)_|🟡 |
  |Fragment Shading Rate| _(VK_KHR_fragment_shading_rate)_|🚧 |
  |Raytracing | _VK_KHR_acceleration_structure_ <br> _VK_KHR_ray_tracing_pipeline_ <br> _VK_KHR_ray_query_ <br> _VK_KHR_pipeline_library_ <br> _VK_KHR_deferred_host_operations_| 🚧|
  |Video Encode| | 🚧|
  |Video Decode| | 🚧|
  |Multi-GPU support | | 🚧 |
  |Headless | | 🚧 |
  |Execution Graphs |(when it becomes available to Vulkan standard) |🚧 |
  
### Lettuce Foundation

- [ ] Render Graph
- [ ] Multi-thread Rendering
- [ ] Async compute
### Lettuce X2D

- [ ] Lights & Shadows
- [ ] Fonts
- [ ] Materials
  - [ ] Glass
  - [ ] Acrylic
- [ ] Reflections
- [ ] Radiosity
- [ ] Particles system
- [ ] Lottie Animations

### Lettuce X3D

- [ ] PBR
- [ ] Lighting System
- [ ] Imports glft2 files (priority)
- [ ] GPU Driving rendering
- [ ] Frustrum culling _per object_
- [ ] Frustrum culling _per region of object_
- [ ] Post Proccessing Effects
  - [ ] Bookeh by distance

### Lettuce Compute

- [ ] Kernels

### hardware-vendors recommendations

[nvidia](https://developer.nvidia.com/blog/vulkan-dos-donts/)
