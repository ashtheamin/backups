#ifndef LIBSDEFS_INCLUDED
#define LIBSDEFS_INCLUDED
    #include "libsdefs.c"
#endif

void helper_print_log(unsigned int object) {
	int log_length = 0;
	if (glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else if (glIsProgram(object)) {
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else {
		printf("helper_print_log(): Not a shader or a program");
		return;
	}

	if (log_length == 0) return;

	char* log = (char*)malloc(log_length);
	printf("log length: %d\n", log_length);
	
	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);
	
	printf("%s", log);
	free(log);
}

unsigned int helper_create_shader(const char* source, GLenum type) {
	if (source == NULL) {
		printf("helper_create_shader(): Error opening '%s'. Exiting.\n", source);
		return 0;
	}
	unsigned int res = glCreateShader(type);
	const char* sources[] = {
		"#version 100\n"
	,
	source };
	glShaderSource(res, 2, sources, NULL);
	//free((void*)source);
	
	glCompileShader(res);
	int compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		helper_print_log(res);
		glDeleteShader(res);
		return 0;
	}
	
	return res;
}