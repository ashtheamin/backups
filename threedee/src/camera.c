#ifndef LIBSDEFS_INCLUDED
#define LIBSDEFS_INCLUDED
    #include "libsdefs.c"
#endif

#ifndef WORLD_INCLUDED
#define WORLD_INCLUDED
    #include "world.c"
#endif

struct mouse {
    float last_x;
    float last_y;
    float sensitivity;
    bool used_before;
};

struct camera {
    vec3 position;
    vec3 front;
    vec3 up;

    float speed;
    float yaw;
    float pitch;

    struct mouse mouse;
};

struct camera* camera_init() {
    struct camera* camera = malloc(sizeof(struct camera));

    memcpy(camera->position, (vec3){0.0, 0.0, 3.0}, sizeof(vec3));
    memcpy(camera->front, (vec3){0.0, 0.0, -1.0}, sizeof(vec3));
    memcpy(camera->up, (vec3){0.0, 1.0, 0.0}, sizeof(vec3));
    camera->yaw = -90.0;
    camera->pitch = 0.0;
    camera->speed = 250;

    camera->mouse.last_x = SCREEN_WIDTH/2;
    camera->mouse.last_y = SCREEN_HEIGHT/2;
    camera->mouse.used_before = false;
    camera->mouse.sensitivity = 0.1;
    
    return camera;
}

// Move camera when mouse to point at a new location when mouse is moved.
void camera_mouse_callback(GLFWwindow* window, double x, double y) {
    (void)window;
    struct camera* camera = (struct camera*)glfwGetWindowUserPointer(window);

    // Set the last x and y mouse positions
    if (camera->mouse.used_before == false) {
        camera->mouse.used_before = true;
        camera->mouse.last_x = x;
        camera->mouse.last_y = y;
    }

    // Get the difference between the last mouse position and current
    float x_offset = x - camera->mouse.last_x;
    float y_offset = y - camera->mouse.last_y;

    // Store the current mouse positions to be the last
    camera->mouse.last_x = x;
    camera->mouse.last_y = y;

    // Multiply the difference in mouse position with sensitivty
    x_offset = x_offset * camera->mouse.sensitivity;
    y_offset = y_offset * camera->mouse.sensitivity;

    // Add the movement difference to the camera's viewing angles.
    camera->yaw = camera->yaw + x_offset;
    camera->pitch = camera->pitch - y_offset;

    // Lock the camera's pitch so it does not go too far up or down
    if (camera->pitch > 89.0) {
        camera->pitch = 89.0;
    }

    if (camera->pitch < -89.0) {
        camera->pitch = -89.0;
    }

    // Calculate the angles that the vector the camera is looking at needs to change with.
    vec3 direction = {0.0, 0.0, 0.0};
    direction[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    direction[1] = sin(glm_rad(camera->pitch));
    direction[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));

    // Add the movement difference to the vector the camera is looking at.
    glm_vec3_normalize_to(direction, camera->front);
}

void camera_free(struct camera* camera) {
    if (camera == NULL) return;
    free(camera);
}