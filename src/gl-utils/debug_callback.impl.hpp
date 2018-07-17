
static glUtils::DebugBehaviourForType debugBehaviourType;

static glUtils::DebugBehaviourForSeverity get_by_debug_msg_type (GLenum type) {
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:               return debugBehaviourType.error;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return debugBehaviourType.deprecated_behavior;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return debugBehaviourType.undefined_behavior;
    case GL_DEBUG_TYPE_PORTABILITY:         return debugBehaviourType.portability;
    case GL_DEBUG_TYPE_PERFORMANCE:         return debugBehaviourType.performance;
    case GL_DEBUG_TYPE_MARKER:              return debugBehaviourType.marker;
    case GL_DEBUG_TYPE_PUSH_GROUP:          return debugBehaviourType.push_group;
    case GL_DEBUG_TYPE_POP_GROUP:           return debugBehaviourType.pop_group;
    default:                                return debugBehaviourType.other;
  }
}

static const char* to_str_type (GLenum type) {
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:               return "error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated_behavior";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "undefined_behavior";
    case GL_DEBUG_TYPE_PORTABILITY:         return "portability";
    case GL_DEBUG_TYPE_PERFORMANCE:         return "performance";
    case GL_DEBUG_TYPE_MARKER:              return "marker";
    case GL_DEBUG_TYPE_PUSH_GROUP:          return "push_group";
    case GL_DEBUG_TYPE_POP_GROUP:           return "pop_group";
    default: return "other";
  }
}

static const char* to_str_source (GLenum type) {
  switch (type) {
    case GL_DEBUG_SOURCE_API:             return "api";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "window_system";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "shader_compiler";
    case GL_DEBUG_SOURCE_APPLICATION:     return "application";
    case GL_DEBUG_SOURCE_THIRD_PARTY:     return "third_party";
    default: return "other";
  }
}

static const char* to_str_severity (GLenum type) {
  switch (type) {
    case GL_DEBUG_SEVERITY_LOW:    return "low";
    case GL_DEBUG_SEVERITY_MEDIUM: return "medium";
    case GL_DEBUG_SEVERITY_HIGH:   return "high";
  }
  return "NOT_VALID";
}

void debug_callback (GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei, const GLchar *message,
    const void *)
{
  using namespace glUtils;
  auto behOpts = get_by_debug_msg_type(type);
  DebugBehaviour beh = severity == GL_DEBUG_SEVERITY_HIGH ? behOpts.high
    : severity == GL_DEBUG_SEVERITY_MEDIUM ? behOpts.medium : behOpts.low;

  // beh = DebugBehaviour::AsDebug;
  if (beh == DebugBehaviour::Skip) {
    return;
  }

  std::string msg = std::string("[GlDebugCallback ")
    + to_str_source(source) + "."
    + to_str_type(type) + "."
    + to_str_severity(severity)
    + " id=" + std::to_string(id) + "]: " + message;

  switch (beh) {
    case DebugBehaviour::Skip: break;
    case DebugBehaviour::AsDebug:  LOGD << msg; break;
    case DebugBehaviour::AsInfo:   LOGI << msg; break;
    case DebugBehaviour::AsError:  LOGE << msg; break;
    case DebugBehaviour::Critical:
      LOGE << msg;
      GFX_FAIL("Received critical message from gl_debug_callback. See above for details");
      break;
  }
}

namespace glUtils {

  void init_debug_callback(const glUtils::DebugBehaviourForType& beh) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // hmm.. TODO remove for optimization? or just use 4.6 no errors mode?
    debugBehaviourType = beh;
    glDebugMessageCallback(debug_callback, nullptr);
  }

} // namespace glUtils
