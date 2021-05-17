# Simply Awesome Game Engine

## Dependencies

- Vulkan
- C11 and C++03 compatible compiler
- meson (Build system)

NOTE: A handy guide on how to install meson [guide](https://mesonbuild.com/Quick-guide.html)

## Build

```sh
git clone --recursive https://github.com/sungrow/sage
python compile.py
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md)

## Roadmap

### 0.1 Release

* [ ] - Job system
* [ ] - Rendering state load from json 
* [ ] - Basic Asset system (Asset app and Asset app output reader in lib)
* [ ] - Compute shaders <!-- Wow. I still haven't implemented them. -->

### 0.2 Release

* [ ] - Rendering state GUI editor
* [ ] - Job system node-like dependency GUI editor
* [ ] - GLTF2.0 support

### 0.3 Release

* [ ] - Game engine GUI
* [ ] - Scripting language

## License

See [LICENSE](LICENSE)

This project has some third-party dependencies, each of which may have independent licensing:

- [fast_obj](https://github.com/thisistherk/fast_obj): Fast obj loader, written in C
- [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): Vulkan Memory Allocator
- [volk](https://github.com/zeux/volk): Meta loader for Vulkan API
- [log.c](https://github.com/rxi/log.c): A simple logging library implemented in C99 
- [meshoptimizer](https://github.com/zeux/meshoptimizer): Mesh optimization library that makes meshes smaller and faster to render
- [glfw](https://github.com/glfw/glfw): A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input 
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers): Vulkan Header files and API registry 
- [hashmap.c](https://github.com/tidwall/hashmap.c): Hash map implementation in C.
- [cJSON](https://github.com/DaveGamble/cJSON): Ultralightweight JSON parser in ANSI C 
- [stb](https://github.com/nothings/stb): stb single-file public domain libraries for C/C++
- [lz4](https://github.com/lz4/lz4): Extremely Fast Compression algorithm

