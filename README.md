# TressFX OpenGL

Direct port of AMD's TressFX hair/fur rendering and simulation technology to OpenGL.



https://github.com/Scthe/TressFX-OpenGL/assets/9325337/0e6c5b6f-c7b3-43f6-8f1e-bf5137a9c87a



*360 view of Sintel's model. Unfortunately, Sintel is not the greatest example to demonstrate TressFX on. Read more in FAQ part of this readme!*


## What is this?

[TressFX](//en.wikipedia.org/wiki/TressFX) is AMD's library used for simulation and rendering of hair. It has been used in commercial games like newest Tomb Raider titles and Deus Ex: Mankind Divided. The library itself is [open source](//github.com/GPUOpen-Effects/TressFX) under GPUOpen initiative. Please visit provided links to get more details.

AMD's version of TressFX uses HLSL (DirectX's shading language) and the Sushi framework. It also provides [fill-the-gaps library](https://github.com/GPUOpen-Effects/TressFX/tree/master/src/TressFX) that is independent of the rest of the source code. The main focus of my project was to provide the implementation for various methods that are needed to render and simulate hair using OpenGL and GLSL. I tried to stick to AMD's version as closely as possible, even if it would have been easier to just write everything from grounds up.

Due to time constraints following techniques are not yet implemented:

* Shortcut for Order Independent Transparency (older Per-Pixel Linked List used instead)
* Skinning (I am not sure how to get this information from Blender)
* Signed Distance Field based collisions (older collision capsules are used instead)
* Shadows
* Realistic rendering (looks like a spaghetti right now, though it is considerably better when viewed in motion on full resolution instead of tiny gif), incl. proper Anti-Aliasing
* Global Shape Constraints from blender-imported models (works fine with AMD's samples). I guess I would have to go deeper into how they generate some data for simulation. ATM. blender plugin is a **massive** hack, so it could be anything from offsets to left/right-hand coordinate system mismatch.
* [Cyrill Crassin's PPLL OIT improvements](http://blog.icare3d.org/2010/07/opengl-40-abuffer-v20-linked-lists-of.html)

There might be still a few bugs that I will hopefully get rid off when I have a little bit more spare time.





https://github.com/Scthe/TressFX-OpenGL/assets/9325337/45fae226-db84-4baf-899e-4656bd4c2b09





## Usage

Each build step has been automated with scripts (found in [bin](bin) directory).

1. Use cmake to generate project. You will need to provide paths for GLM, SDL2 and GLAD. Script: [generate_makefile.clang.bat](bin/generate_makefile.clang.bat).
    PS. I've used clang, not tested on other compilers.

2. Generate shaders. I've written simple python script that processes .glsl files for `#pragma include "xxx.glsl"` and textually replaces this line with content of the file. You can even do this by hand. Script: [build_glsl.py](bin/build_glsl.py). At the end of this step You should have following files:

    * src\shaders\generated\gl-tfx\ppll_build.frag.glsl
    * src\shaders\generated\gl-tfx\ppll_build.vert.glsl
    * src\shaders\generated\gl-tfx\ppll_resolve.frag.glsl
    * src\shaders\generated\gl-tfx\ppll_resolve.vert.glsl
    * src\shaders\generated\gl-tfx\sim0_IntegrationAndGlobalShapeConstraints.comp.glsl
    * src\shaders\generated\gl-tfx\sim1_VelocityShockPropagation.comp.glsl
    * src\shaders\generated\gl-tfx\sim2_LocalShapeConstraints.comp.glsl
    * src\shaders\generated\gl-tfx\sim3_LengthConstraintsWindAndCollision.comp.glsl
    * src\shaders\generated\gl-tfx\sim3_TMP_LengthConstraintsWindAndCollision.comp.glsl
    * src\shaders\generated\gl-tfx\sim4_UpdateFollowHairVertices.comp.glsl

    PS. `build_glsl.py` also calls `bin\glslcompiler\GLSLCompiler.exe`, which is small shader compiler. It takes .glsl path as first argument and does test compile. This tool is useful for development and is **not** included in this repo. Just comment this line, You should not need it (TressFx.exe will fail anyway if shader does not compile).

3. Build: `make`. Script: [build_and_run.bat](bin/build_and_run.bat).
    PS. this script also invokes `build_glsl.py` and runs `bin\TressFx.exe` upon successful compile. All-in-one.

4. Run `bin\TressFx.exe`


TL;DR: It's just 2 scripts to run project from scratch:

1. [generate_makefile.clang.bat](bin/generate_makefile.clang.bat)
2. [build_and_run.bat](bin/build_and_run.bat) that calls:
    1. [build_glsl.py](bin/build_glsl.py)
    1. `make` on cmake-generated makefile
    1. `bin\TressFx.exe` - to run executable


## FAQ

**Q: Required OpenGL version?**

A: Use 4.5+. Technically direct state access(DSA), shader storage buffer object(SSBO), compute shaders, image load/store, atomic ops are required. They could be used through extensions, but You would probably have to write support for them by hand. Just use 4.5+ and it should work.

**Q: Some warnings during compilation?**

A: They are from AMD's code that is included. AMD had used visual studio, so there were a few things I had to change to make it compile under clang. See next question.

**Q: What changes were needed compared to AMD's library?**

A: [Complete list](libs/amd_tressfx/src/Readme.md). I tried to keep the changes to minimum, even if it included having warnings during compilation. My own API is also modeled to be as close as possible.

**Q: Is this a Blender plugin?**

A: [Sure](assets/sintel_lite_v2_1/tfx_exporter.py). It is very bare-bones ATM, I will probably make a separate github repo for it. It does not support .tfxbone (skinning) and per-strand uv, but it is enough for simulation and basic rendering. Should work with AMD's viewer, although I did not test it.

**Q: How to load new models?**

A: Authoring new models for TressFX is actually quite complicated. First, there is the scale. There are certain simulation steps that require models to have roughly comparable scale. Good example is wind: displacing hair strand that has length of 3 units is vastly different to hair strand 300 units long. Sintel's model is about 50x50x50 blender units.

Another thing is tweaking all simulation and rendering parameters. E.g. AMD's library calculates random vectors around the guide hair where follow hair will be placed. Since You do not have any input in this (unless You modify AMD's library, which I tried not to do), it may be required to tweak it later.

There are also collision capsules that are really hard to get right. When hair near root intersects with collision capsule it is automatically 'pushed away'. Collision resolve has the highest priority and that results in colliding part of the hair strand just ignoring any other simulation forces. One could make the capsules smaller, but that leads to penetration of the object.

Fortunately, just getting it to display requires only to import the model from Blender / Maya.

**Q: Available models?**

A: Found in [assets](assets):

* Sintel from Blender institute
* Ratboy's Mohawk from original AMD TressFX

**Q: Sintel? How cool!**

A: Well, I made a rule to **not modify original sintel lite hair model**. Absolutely no content authoring, even small fixes. As a result, there are a few issues, which can probably be noticed when watching animations above. The biggest issue is wind that blows from one side and pushes half of the hair strands 'into' the head model. This is resolved as a collision with collision capsule and fixed by overriding strand position. Which in turn results in half of the hair just ignoring wind altogether. I think one could solve this by using multiple wind sources, instead of averaging? Or just properly author the model.

We can compare this to models from commercial games that utilize TressFX:

* Adam Jensen (Deus Ex: Mankind Divided) has very short hair.
* Lara Croft's (Tomb Raider 2013, Rise of the Tomb Raider) ponytail acts more like a ribbon that has considerably simpler interactions with rest of the model (though I assume it still was a nightmare to get the parameters right). The rest of the hairstyle is rather stiff in comparison. This is in stark contrast to Sintel, where whole hair is purely under the control of the simulation.
  PS. Rise of The Tomb Raider used evolution of TressFX 3.0 called [PureHair](https://www.youtube.com/watch?v=wrhSVcZF-1I). In the video You can clearly see what an experienced artist can do with a system like TressFX. Interestingly, not all hair is simulated, but only a few strands in key places (like bangs). It still gives a very dynamic feeling.

**Q: What is glUtils?**

A: **VERY** thin wrapper around OpenGL. Thin as in 'You still have to write most of gl- calls anyway' thin. It mostly deals with resource creation. It was made to make OpenGL api more functional. Good example is [glUtils::apply_draw_parameters](src/gl-utils/draw/apply_parameters.impl.hpp) that takes [DrawParameters](include/gl-utils/draw/parameters.hpp) structure as an argument. Feel free to look around, there are some comments about how does OpenGL work out there that You may find interesting.

**Q: What to do if I have a problem?**

A: In [main()](src/main.cpp) set log level to trace: `logger::Log::ReportingLevel = logger::Trace;`. It will generate A LOT of text into console. Also, the code is CRAMMED with assertions. And if the assertion is not clear enough, just go to the code line (printed with assertion message) and there is usually a comment about what could have gone wrong.

**Q: Your PC?**

A: GTX 1050 Ti, Driver 398.36



## Honorable mentions and other 3rd party stuff

* AMD for TressFX
* [GLM](https://glm.g-truc.net/0.9.9/index.html)
* [SDL2](https://www.libsdl.org/download-2.0.php)
* [GLAD](https://github.com/Dav1dde/glad)
* [imgui](https://github.com/ocornut/imgui) <3
* [tiny_obj_loader](https://github.com/syoyo/tinyobjloader)
* [apitrace](http://apitrace.github.io/)
* [RenderDoc](https://renderdoc.org/)
* [Blender](https://www.blender.org/), [Blender Institute](https://www.blender.org/institute/) <3
    Sintel's model under [CC 3.0](https://durian.blender.org/sharing/), character was simplified into bust. © copyright Blender Foundation | durian.blender.org


## Licencing

Certain parts of this project are under [AMD's licence](https://github.com/GPUOpen-Effects/TressFX/blob/master/LICENSE.txt) (which is MIT) - noted usually as licence file in directory or in file header. Rest is under licence of this project (which is also MIT).
