# Requeriments

- vcpkg
- Vulkan SDK
- CMake 3.30 or greater
- C++ compiler with C++ 20 support

## Prepare the poject to build

### Step 1
Clone the repository.
```cmd
git clone --recurse-submodules https://github.com/PieroCastillo/Lettuce.git
```
### Step 2
Update vcpkg baseline.
```cmd
vcpkg x-update-baseline
```
### Step 3
Configure the project with the generator of your preference (in this case with Visual Studio).
```cmd
cmake --preset=vcpkg -G "Visual Studio 17 2022"
```
## Build
Build with cmake.
```cmd
cmake --build build
```