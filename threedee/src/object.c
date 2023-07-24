#ifndef LIBSDEFS_INCLUDED
#define LIBSDEFS_INCLUDED
    #include "libsdefs.c"
#endif

#ifndef SHADERS_INCLUDED
#define SHADERS_INCLUDED
#include "shader.c"
#endif

struct object {
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    vec3 position;
    vec3 scale;
    float vertices[180];
    unsigned int indices[6];
    char* name;
    struct object* next;
    struct shader* shader_list;
};

struct object* object_init() {
    struct object* object = malloc(sizeof(struct object));
    object->name = strdup("");
    if (object == NULL) {
        printf("object_init(): Failed to create object. Returning.\n");
        return NULL;
    }
    glm_vec3_copy((vec3){0.0, 0.0, 0.0}, object->position);
    glm_vec3_copy((vec3){1.0, 1.0, 1.0}, object->scale);

    object->next = NULL;
    object->shader_list = NULL;
    return object;
}

void object_free(struct object* object) {
    if (object == NULL) return;
    if (object->shader_list != NULL) shader_list_free(object->shader_list);
    if (object->name != NULL) free(object->name);
    free(object);
}

void object_list_free(struct object* object) {
    if (object == NULL) return;
    struct object* temp = object;
    struct object* next;

    while (temp != NULL) {
        next = temp->next;
        free(temp);
        temp = next;
    }
}

struct object* object_init_cube(struct shader* shader) {
    struct object* object = malloc(sizeof(struct object));
    if (object == NULL) {
        printf("object_init(): Failed to create object. Returning.\n");
        return NULL;
    }
    object->name = strdup("cube");

    memcpy(object->vertices, (float [180]){
     -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    }, 180*sizeof(float));

    glBindVertexArray(object->VAO);
    glGenVertexArrays(1, &object->VAO);

    glGenBuffers(1, &object->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, object->VBO);
    glBindVertexArray(object->VAO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(object->vertices), object->vertices, GL_STATIC_DRAW);

    object->shader_list = NULL;
    struct shader_attribute* attribute = shader->attributes;

    int coord3d = 0;
    while (attribute != NULL) {
        if (strcmp(attribute->name,"coord3d") == 0) {
            coord3d = attribute->attribute;
        }
        attribute = attribute->next;
    }

    glEnableVertexAttribArray(coord3d);
    glVertexAttribPointer(coord3d, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    object->next = NULL;
    glm_vec3_copy((vec3){0.0, 0.0, 0.0}, object->position);
    glm_vec3_copy((vec3){1.0, 1.0, 1.0}, object->scale);

    return object;
}

