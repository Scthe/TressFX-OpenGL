#define CONCAT(a, b) a ## b

#define STRUCTURED_BUFFER(BINDING, TYPE, INTROSPECT_NAME) \
	layout(std430, binding=BINDING) \
	buffer INTROSPECT_NAME { \
		TYPE CONCAT(INTROSPECT_NAME, _)[]; \
	};
