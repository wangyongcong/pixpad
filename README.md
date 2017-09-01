pixpad
======
Computer graphics testbed, with a software renderer Sparrow & utility tools.

Build
=====
Dependent 3rd libs:
* boost
* openexr (IlmBase)

### Windows
Use CMake (at least 3.3) to configure the build.

Sparrow
====
A multi-threading software renderer.
- C/S model with an asynchronous command queue.
- Programmable pipeline with multi-threading support. Vertex & fragment shading is run in parallel to speed up.
- Flexible material system. It's easy to experiment variant shaders. 
- Depth test
- Texture mapping
- MIP-mapping
	
Roadmap
====
### Features
- anti-aliasing (SMAA)
- detailed pipeline metrics
- deferred shading
- ray tracer
- and more...
### Optimization
- tile based memory layout
- SIMD support
- and more...

Gallery
====
<img alt="depth test" src="gallery/depth_test.png" width=336 height=189>
<img alt="textured box" src="gallery/textured_box.png" width=336 height=189>
