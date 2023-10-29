# Requeriments

- vcpkg
- Vulkan SDK
- CMake 3.26 or greater

## Prepare the poject to build

### Step 1
Install SDL2 package (use x64 triplet).
For example, if you are using windows run the follow command:

```cmd
    vcpkg install SDL2 --triplet x64-windows
```
### Step 2 
set an enviroment variable called ´´´VC_PKG_CMAKE´´´ that contains the file ´´´vcpkg.cmake´´´
Example:
if that file is contained in the path ´´´C:/dev/vcpkg/scripts/buildsystems/´´´ so 
the value of the variable ´´´VC_PKG_CMAKE´´´  is ´´´C:/dev/vcpkg/scripts/buildsystems/´´´
### Step 3
run the follow command in the console:
```cmd
cmake -S . -B bin
```
## Build
To build run the follow command:
```cmd
cmake --build bin 
```