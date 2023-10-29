add_library(LettuceCore SHARED
			Factory.cpp Device.cpp GPU.cpp Lettuce.Core.h)

target_link_libraries(LettuceCore PRIVATE Vulkan::Vulkan Vulkan::Headers)
target_compile_features(LettuceCore PUBLIC cxx_std_11)
target_include_directories(LettuceCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${Vulkan_INCLUDE_DIRS})