#pragma once

#include "../pch.hpp"
#include "pch.hpp"

#include <SDL2/SDL.h>
#include <initializer_list>
#include <vector>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "draw/parameters.hpp"
#include "shader/shader.hpp"
#include "texture/texture.hpp"
#include "buffer.hpp"
#include "sync.hpp"
#include "VAO.hpp"
#include "window.hpp"


// TODO assumes that GFX_FAIL will crash the program/throw exception


namespace glUtils {

	typedef const char*const UniformName;

	/// Window (using SDL, GLAD)
	GlWindow create_window(const WindowInitOpts& opt);
	void destroy(GlWindow&);

	/// Buffers
	RawBuffer malloc(const u32 bytes, const BufferUsagePattern usage_pattern);
	void invalidate (const RawBuffer&, const BufferRange);
	u32 write (const RawBuffer&, const BufferRange, const void*const);
	u32 read (const RawBuffer&, const BufferRange, void*const);
	u32 copy_to(const RawBuffer& src, const RawBuffer& dest, const BufferRange src_range, const u32 dest_offset);
	void destroy(RawBuffer&);


	// VAO
	VAO create_vao(std::initializer_list<VertexAttribute>);
	void destroy(VAO&);


	// Shaders
	Shader create_shader(const ShaderTexts&, ShaderErrorsScratchpad&);
	void destroy(Shader&);


  // Textures
	void allocate_texture(Texture&, bool apply_texture_options=true);
	void apply_texture_options(const Texture&, const TextureOpts&);
	void destroy(Texture&);


  // Framebuffer objects (FBO)
  // FBO create_fbo(std::initializer_list<FboAttachment>);


  // Draw
	void apply_draw_parameters(const DrawParameters& new_params, const DrawParameters*const old_state=nullptr);
	// void draw_indexed(const Shader&, const Fbo&, const DrawIndexedParams&, etc.);  // TODO calculate glViewport from fbo textures
	// void draw();


  // Synchronization
  // @see docs in sync.hpp
  void async_write_barrier (AsyncWriteableResource_);


	// Shader resources
	// (only UniformBuffer, AtomicCounterBuffer, ShaderStorageBuffer, TransformFeedbackBuffer)
	// NOTE: UBO use only offset from provided range, as we can get exact size of ubo
	// struct using introspection API. OTOH, SSBO can have not limited array
	// as last element - in such case we use both provided offset and size
	void set_shader_buffer(Shader&, GLuint gl_binding, UniformName, const RawBuffer&, BufferRange);
	void set_shader_buffer(Shader&, GLuint gl_binding, ShaderBlock&, const RawBuffer&, BufferRange);
	void set_shader_atomic(const Shader&, u32 gl_binding, const RawBuffer&, u32 offset);

	// Uniforms
	// NOTE: these functions may fail silently. The reason is quite simple:
	// we use OpenGL's introspect API to get location of uniform in shader.
	// But, if uniform is NOT USED, introspect API will not return any value
	// (as changing the uniform value would have no effect). It is quite
	// common to set uniforms that are not used, which would result in
	// a lot of errors printed to console. E.g. during debugging one would
	// comment half of the code, leaving much of the parameters just 'hanging'.
	// Similar solution of ignoring errors is used by Three.js.
	// If You really need this functionality, use force flag. This will:
	//   a) crash if uniform does not exist
	//   b) crash if provided wrong data type e.g. provided float,
	//      but introspection API says that it is uint in GLSL
	//

	u32 set_uniform(const Shader&, const UniformName,   void* data, bool force=false);
	u32 set_uniform(const ShaderVariable&, void* data); // we know variable exists, we know the types will match (as we will deduct 'data' type based on ShaderVariable)

	#define UNIFORM_MACRO(CPP_TYPE, GL_TYPE, GL_SET_FN, VALUE_EXTRACT) \
	  void set_uniform(const Shader&, const UniformName,   const CPP_TYPE, bool force=false); \
	  void set_uniform(const ShaderVariable&, const CPP_TYPE, bool force=false);
	#define UNIFORM_MACRO_MAT(CPP_TYPE, GL_TYPE, GL_SET_FN, VALUE_EXTRACT) \
	  void set_uniform(const Shader&, const UniformName,   const CPP_TYPE, bool transpose=false, bool force=false); \
	  void set_uniform(const ShaderVariable&, const CPP_TYPE, bool transpose=false, bool force=false);
	#include "uniform.types.hpp"
  #undef UNIFORM_MACRO
	#undef UNIFORM_MACRO_MAT

	template<typename T>
	T uniform_convert(void* data, u32& consumed_bytes) {
	  consumed_bytes = sizeof(T);
	  T* t_p = static_cast<T*>(data);
	  return *t_p;
	}

	// Debug
	enum DebugBehaviour {
	  Skip, AsDebug, AsInfo, AsError, Critical
	};

	struct DebugBehaviourForSeverity {
		DebugBehaviour low = DebugBehaviour::Skip;
		DebugBehaviour medium = DebugBehaviour::Skip;
		DebugBehaviour high = DebugBehaviour::Skip;
		inline void set_all(DebugBehaviour db) {low = medium = high = db; }
	};

	struct DebugBehaviourForType {
		DebugBehaviourForSeverity error;
		DebugBehaviourForSeverity deprecated_behavior;
		DebugBehaviourForSeverity undefined_behavior;
		DebugBehaviourForSeverity portability;
		DebugBehaviourForSeverity performance;
		DebugBehaviourForSeverity marker;
		DebugBehaviourForSeverity push_group;
		DebugBehaviourForSeverity pop_group;
		DebugBehaviourForSeverity other;
	};

	void init_debug_callback(const DebugBehaviourForType&);

}
