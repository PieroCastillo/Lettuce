add_executable(TriangleSample main.cpp)
target_compile_features(TriangleSample PUBLIC cxx_std_11)
target_link_libraries(TriangleSample PRIVATE LettuceCore SDL2::SDL2) 