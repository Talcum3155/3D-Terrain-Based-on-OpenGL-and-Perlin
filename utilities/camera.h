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
#include <iostream>

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
    const float SPEED = 200.0f;
    const float SENSITIVITY = 0.1f;
    const float ZOOM = 45.0f;

    class camera {
    public:
        bool enable_mouse_movement = true;

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
        bool first_move = true;
        // store mouse last position to compute yaw and pitch
        glm::vec2 last_pos;

        camera(glm::vec2 last_pos, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
               glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f),
               float yaw = YAW, float pitch = PITCH);

        camera(float pos_x, float pos_y, float pos_z,
               float world_up_x, float world_up_y, float world_up_z,
               float yaw, float pitch, glm::vec2 last_pos);

        [[nodiscard]] inline glm::mat4 get_view_matrix() const;

        [[nodiscard]] inline glm::mat4
        get_projection_matrix(int width, int height, float z_near = 0.1f, float z_far = 100.0f) const;

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
    camera::get_view_matrix() const {
        // first parameter: camera position
        // second parameter: look direction
        // third parameter: world up direction
        return glm::lookAt(position, position + camera::forward, camera::up);
    }

    /**
     * Return the projection matrix based on window resolution, near plane, far plane
     * @param width
     * @param height
     * @param z_near near plane
     * @param z_far far plane
     * @return projection matrix
     */
    inline glm::mat4
    camera::get_projection_matrix(int width, int height, float z_near, float z_far) const {
        return glm::perspective(glm::radians(zoom),
                                static_cast<float>(width) / static_cast<float>(height),
                                z_near, z_far);
    }
}


#endif //INC_3DPERLINMAP_CAMERA_H
