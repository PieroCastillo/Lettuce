# Requeriments

- vcpkg
- Vulkan SDK
- CMake 3.26 or greater

## Prepare the poject to build

### Step 1
Install SDL2 package (use x64 triplet).
For example, if you are using windows run the follow command:

```cmd
vcpkg install glfw3 
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