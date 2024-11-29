# Lettuce

Lettuce is a rendering Vulkan-based library.
The focus of Lettuce is be a library of real-time graphics oriented to design and engineering with the better 
performance as possible.

### Docs
 Tools:
  -  Doxygen + Doxygen Awesome, for API reference
  -  Docsify for Documentation

## Components 

### Lettuce Core
- [x] Instance
- [x] GPU
- [ ] Multi-GPU support
- [ ] Headless 
- [x] Swapchain
- [x] CommandList (partial)
- [x] GraphicsPipeline
- [x] ComputePipeline
- [x] Mesh shading
- [x] Buffer (bad, this impl uses 1 allocation per buffer)
- [x] Textures
- [x] Samplers
- [x] Dynamic states
- [x] Timeline Semaphores
- [x] Synchronization2
- [x] Descriptor Indexing
- [ ] Buffer Device Address
- [ ] VK_EXT_memory_budget
- [ ] VK_EXT_conservative_rasterization
- [ ] VK_KHR_fragment_shading_rate
- [ ] Indirect Drawing
- [ ] Execution Graphs (when it becomes available to Vulkan standard)
- [ ] Video
- [ ] HLSL
- [ ] SLang
- [ ] MemoryBlock (various buffers into 1 allocation)
### Lettuce Foundation
- [ ] Render Graph
- [ ] Multi-thread Rendering
- [ ] Async compute
- [ ] Frustrum culling _per object_
- [ ] Frustrum culling _per region of object_
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
### Lettuce Compute

### hardware-vendors recommendations

[nvidia](https://developer.nvidia.com/blog/vulkan-dos-donts/)