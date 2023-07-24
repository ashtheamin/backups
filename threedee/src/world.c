#ifndef LIBSDEFS_INCLUDED
#define LIBSDEFS_INCLUDED
    #include "libsdefs.c"
#endif

#ifndef SHADERS_INCLUDED
#define SHADERS_INCLUDED
    #include "shader.c"
#endif

#ifndef OBJECT_INCLUDED
#define OBJECT_INCLUDED
    #include "object.c"
#endif

#include "camera.c"

enum world_status {WORLD_STATUS_QUIT, WORLD_STATUS_RUNNING};
struct world {
    GLFWwindow* window;
    enum world_status status;
    struct object* object_list;
    struct shader* shader_list;
    struct camera* camera;
};

void world_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}  

struct world* world_init() {
    struct world* world = malloc(sizeof(struct world));
    if (world == NULL) {
        printf("world_init(): Failed to init world. Exiting.\n");
        return NULL;
    }

    if (glfwInit() != GLFW_TRUE) {
        printf("world_init(): Failed to init GLFW. Exiting.\n");
        free(world);
        return NULL;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  
    world->window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Window", NULL, NULL);
    if (world->window == NULL) {
        printf("world_init(): Failed to initialise GLFW Window. Returning.\n");
        free(world);
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(world->window);
    if (glewInit() != GLEW_OK) {
        printf("world_init(): Failed to initalise GLEW. Exiting.\n");
        free(world);
        glfwTerminate();
        return NULL;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glfwSetFramebufferSizeCallback(world->window, world_framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);

    world->camera = camera_init();
    glfwSetWindowUserPointer(world->window, world->camera);
    glfwSetInputMode(world->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(world->window, camera_mouse_callback);

    world->shader_list = NULL;
    world->object_list = NULL;

    world->shader_list = shader_init_default();
    world->object_list = object_init_cube(world->shader_list);
    
    world->object_list->next = object_init_cube(world->shader_list);
    glm_vec3_copy((vec3){3.0, 3.0, 0.0},world->object_list->next->position);
    glm_vec3_copy((vec3){0.5, 3.0, 0.5}, world->object_list->next->scale);

    return world;
}

void world_input(struct world* world) {
    float speed = 0.4;
    //world->camera->speed * world->timing.delta_time;
    vec3 updated_position = {0.0, 0.0, 0.0};

    if (glfwGetKey(world->window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(world->camera->front, speed, updated_position);
        glm_vec3_add(world->camera->position, updated_position, world->camera->position);
    }

    if (glfwGetKey(world->window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(world->camera->front, speed, updated_position);
        glm_vec3_negate(updated_position);
        glm_vec3_add(world->camera->position, updated_position, world->camera->position);
    }

    if (glfwGetKey(world->window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_cross(world->camera->front, world->camera->up, updated_position);
        glm_vec3_normalize(updated_position);
        glm_vec3_scale(updated_position, speed, updated_position);
        glm_vec3_negate(updated_position);
        glm_vec3_add(updated_position, world->camera->position, world->camera->position);
    }

    if (glfwGetKey(world->window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_cross(world->camera->front, world->camera->up, updated_position);
        glm_vec3_normalize(updated_position);
        glm_vec3_scale(updated_position, speed, updated_position);
        glm_vec3_add(updated_position, world->camera->position, world->camera->position);
    }
}

void world_render(struct world* world) {
    if (world == NULL) return;
    glClearColor(0.2, 0.2, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    struct shader* shader = world->shader_list;
    glUseProgram(shader->program);
    
    int uniform_model;
    int uniform_view;
    int uniform_projection;

    struct shader_uniform* uniform = shader->uniforms;
    while (uniform != NULL) {
        if (strcmp(uniform->name, "model") == 0) {
            uniform_model = uniform->uniform;
        }
        if (strcmp(uniform->name, "view") == 0) {
            uniform_view = uniform->uniform;
        }
        if (strcmp(uniform->name, "projection") == 0) {
            uniform_projection = uniform->uniform;
        }
        uniform = uniform->next;
    }


    struct object* object = world->object_list;
    while (object != NULL) {
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        mat4 view = GLM_MAT4_IDENTITY_INIT;
        mat4 projection = GLM_MAT4_IDENTITY_INIT;

        glm_perspective(glm_rad(45.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100000.0f, projection);

        vec3 lookingat = {0.0, 0.0, 0.0};
        glm_vec3_add(world->camera->position, world->camera->front, lookingat);
        glm_lookat(world->camera->position, lookingat, world->camera->up, view);

        glm_translate(model, object->position);
        glm_scale(model, object->scale);

        glUniformMatrix4fv(uniform_model, 1, GL_FALSE, model[0]);
        glUniformMatrix4fv(uniform_view, 1, GL_FALSE, view[0]);
        glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, projection[0]);

        glBindVertexArray(object->VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        object = object->next;
    }  
}

void world_loop(void* world_arg) {
    struct world* world = (struct world*)world_arg;
    if (world == NULL) return;
    world_input(world);
    world_render(world);
    glfwSwapBuffers(world->window);
    glfwPollEvents();
}

void world_quit(struct world* world) {
    if (world == NULL) return;
    if (world->camera != NULL) camera_free(world->camera);
    if (world->object_list != NULL) object_list_free(world->object_list);
    if (world->shader_list != NULL) shader_list_free(world->shader_list);
    free(world);
    glfwTerminate();
}
