//
// Created by Tarowy on 2023-11-15.
//

#ifndef INC_3DPERLINMAP_CAMERA_H
#define INC_3DPERLINMAP_CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <algorithm>

namespace utilities {

    enum camera_movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    // Default camera values
    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float SPEED = 2.5f;
    const float SENSITIVITY = 0.1f;
    const float ZOOM = 45.0f;

    class camera {
    public:
        // camera vectors
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 up;
        glm::vec3 right;
        const glm::vec3 worldUp;

        // euler Angles
        float yaw;
        float pitch;

        // camera options
        float movement_speed;
        float mouse_sensitivity;
        float zoom;

        // control various
        bool first_move;
        glm::vec2 last_pos;

        camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
               glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
               float yaw = YAW, float pitch = PITCH);

        camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

        inline glm::mat4 get_view_matrix();

        void process_key_input(camera_movement direction, float delta_time);

        void process_mouse_movement(float x_offset, float y_offset, bool constrain_pitch = false);

        void process_mouse_scroll(float y_offset);

    private:
        void update_camera_vectors();
    };

    /**
     * Processes input received from a mouse scroll-wheel event.
     * Only requires input on the vertical wheel-axis
     * @param y_offset
     */
    inline void
    camera::process_mouse_scroll(float y_offset) {
        zoom = std::clamp(zoom - static_cast<float>(y_offset), 1.0f, 45.0f);
    }

    /**
     * Return the coordinate transformation matrix with the camera as the origin.
     * @return coordinate transformation matrix
     */
    inline glm::mat4
    camera::get_view_matrix() {
        // first parameter: camera position
        // second parameter: look direction
        // third parameter: world up direction
        return glm::lookAt(position, position + camera::forward, camera::up);
    }
}


#endif //INC_3DPERLINMAP_CAMERA_H
