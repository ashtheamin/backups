#ifndef LIBSDEFS_INCLUDED
#define LIBSDEFS_INCLUDED
    #include "libsdefs.c"
#endif

struct shader_attribute {
    int attribute;
    char* name;
    struct shader_attribute* next;
};

struct shader_uniform {
    int uniform;
    char* name;
    struct shader_uniform* next;
};

struct shader {
    struct shader_attribute* attributes;
    struct shader_uniform* uniforms;
    unsigned int program;
    struct shader* next;
};

void shader_append_attributes(struct shader* shader, char* name) {
    if (shader == NULL) return;
    if (shader->attributes == NULL) {
        shader->attributes = malloc(sizeof(struct shader_attribute));
        shader->attributes->name = strdup(name);
        shader->attributes->attribute = glGetAttribLocation(shader->program, name);
        shader->attributes->next = NULL;
        return;
    }

    struct shader_attribute* temp = shader->attributes;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = malloc(sizeof(struct shader_attribute));
    temp->next->next = NULL;
    temp->next->attribute = shader->attributes->attribute = glGetAttribLocation(shader->program, name);
    temp->next->name = strdup(name);
}

void shader_append_uniforms(struct shader* shader, char* name) {
    if (shader == NULL) return;
    if (shader->uniforms == NULL) {
        shader->uniforms = malloc(sizeof(struct shader_uniform));
        shader->uniforms->name = strdup(name);
        shader->uniforms->uniform = glGetUniformLocation(shader->program, name);
        shader->uniforms->next = NULL;
        return;
    }

    struct shader_uniform* temp = shader->uniforms;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = malloc(sizeof(struct shader_uniform));
    temp->next->next = NULL;
    temp->next->uniform = glGetUniformLocation(shader->program, name);
    temp->next->name = strdup(name);
}

struct shader* shader_init_default() {
    struct shader* shader = malloc(sizeof(struct shader));
    if (shader == NULL) {
        printf("shader_init(): malloc Failed to create shader. Exiting.\n");
        return NULL;
    }

    shader->attributes = NULL;
    shader->next = NULL;

    char* vertex_shader_source = 
    "attribute vec3 coord3d;                      "
    "uniform mat4 model;                          "
    "uniform mat4 view;                           "
    "uniform mat4 projection;                     "
		"void main(void) {                        "
		"  gl_Position = projection * view * model * vec4(coord3d, 1.0); "
	"}";

    char* fragment_shader_source = 
    "void main(void) {        "
		"  gl_FragColor[0] = gl_FragCoord.x/1280.0; "
		"  gl_FragColor[1] = gl_FragCoord.y/720.0; "
		"  gl_FragColor[2] = 0.5; "
	"}";

    unsigned int vertex_shader = helper_create_shader(vertex_shader_source,GL_VERTEX_SHADER);
    unsigned int fragment_shader = helper_create_shader(fragment_shader_source, GL_FRAGMENT_SHADER);

    shader->program = glCreateProgram();
    glAttachShader(shader->program, vertex_shader);
    glAttachShader(shader->program, fragment_shader);
    glLinkProgram(shader->program);
    helper_print_log(shader->program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    shader->attributes = NULL;
    shader->uniforms = NULL;

    shader_append_attributes(shader, "coord3d");
    shader_append_uniforms(shader, "model");
    shader_append_uniforms(shader, "view");
    shader_append_uniforms(shader, "projection");
    return shader;
}

void shader_free(struct shader* shader) {
    if (shader == NULL) return;
    if (shader->attributes != NULL) {
        struct shader_attribute* temp = shader->attributes;
        struct shader_attribute* next;
        while (temp != NULL) {
            next = temp->next;
            free(temp);
            temp = next;
        }
    }
    if (shader->uniforms != NULL) {
        struct shader_uniform* temp = shader->uniforms;
        struct shader_uniform* next;
        while (temp != NULL) {
            next = temp->next;
            free(temp);
            temp = next;
        }
    }
    glDeleteProgram(shader->program);
    free(shader);
}

void shader_list_free(struct shader* shader) {
    if (shader == NULL) return;
    struct shader* temp = shader;
    struct shader* next;

    while (temp != NULL) {
        next = temp->next;
        free(temp);
        temp = next;
    }
}
