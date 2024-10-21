# Requeriments

- vcpkg
- Vulkan SDK
- CMake 3.26 or greater

## Prepare the poject to build

### Step 1
Install vcpkg packages with the follow command:

```cmd
vcpkg install glfw3 glm volk vulkan vulkan-memory-allocator vulkan-sdk-components tinygltf 
```
### Step 2 

### Step 3
run the follow command in the console:
```cmd
cmake --preset=vcpkg -G "Visual Studio 17 2022"
```
## Build
To build run the follow command:
```cmd
cmake --build build
```