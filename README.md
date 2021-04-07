# Simply Awesome Game Engine

## Dependencies

- Vulkan
- C99 and C++03 compatible compiler
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

* [ ] - Superstruct to contain all of the applictation state and clean it (or parts of it of a select type) up upon a cleanup function call. Possibly repurpose SgApp (Ex Funcs: sgClearApp, sgClearResouces, sgClearMaterials)

[//]:#(Note: Mb not a good idea as it would lead to hanging pointers to a freed data and unobvious state change (Function only needs a pointer to an app to change any state it wants. If we allow the users to be lazy, they will be).)

* [ ] - Material and MaterialRenderObject serialization
* [ ] - Resource update in bulk with a semaphore
* [ ] - Fix mesh load
* [ ] - Programmable render pass interface
* [ ] - Make an actual roadmap that is not filled with junk
* [ ] - Animations
* [ ] - Sound
* [ ] - Job system

## License

See [LICENSE](LICENSE)

This project has some third-party dependencies, each of which may have independent licensing:

- [fast_obj](https://github.com/thisistherk/fast_obj): Fast obj loader, written in C
- [vma](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): Vulkan Memory Allocator
- [volk](https://github.com/zeux/volk): Meta loader for Vulkan API
- [log.c](https://github.com/SanderMertens/flecs): 
- [meshoptimizer](https://github.com/zeux/meshoptimizer):
- [glfw](https://github.com/glfw/glfw):
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers):
- [hashmap.c](https://github.com/tidwall/hashmap.c):
- [cJSON](https://github.com/DaveGamble/cJSON):
- [stb](https://github.com/nothings/stb):

