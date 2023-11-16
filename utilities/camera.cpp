//
// Created by Tarowy on 2023-11-15.
//

#include "camera.h"

namespace utilities {

    camera::camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
            : position(position), worldUp(up),
              yaw(yaw), pitch(pitch),
              forward(glm::vec3(0.0f, 0.0f, -1.0f)),
              movement_speed(SPEED), mouse_sensitivity(SENSITIVITY), zoom(ZOOM) {
        update_camera_vectors();
    }

    camera::camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
            : position(posX, posY, posZ), worldUp(upX, upY, upZ),
              yaw(yaw), pitch(pitch),
              forward(glm::vec3(0.0f, 0.0f, -1.0f)),
              movement_speed(SPEED), mouse_sensitivity(SENSITIVITY), zoom(ZOOM) {
        update_camera_vectors();
    }

    /**
     * Processes input received from any keyboard-like input system.
     * @param direction
     * @param delta_time
     */
    void
    camera::process_key_input(camera_movement direction, float delta_time) {
        float velocity = movement_speed * delta_time;

        switch (direction) {
            case FORWARD:
                position += forward * velocity;
                break;
            case BACKWARD:
                position -= forward * velocity;
                break;
            case LEFT:
                position -= right * velocity;
                break;
            case RIGHT:
                position += right * velocity;
                break;
        }
    }

    /**
     * Processes input received from a mouse input system.
     * @param x_offset
     * @param y_offset
     * @param constrain_pitch
     */
    void
    camera::process_mouse_movement(float x_offset, float y_offset, bool constrain_pitch) {
        x_offset *= mouse_sensitivity;
        y_offset *= mouse_sensitivity;

        yaw += x_offset;
        // constrain the max and min value of pitch
        pitch = constrain_pitch ? std::clamp(pitch + y_offset, -89.0f, 89.0f) : pitch;

        // update Front, Right and Up Vectors using the updated Euler angles
        update_camera_vectors();
    }

    /**
     * Calculates the front vector from the Camera's (updated) Euler Angles
     */
    void
    camera::update_camera_vectors() {
        forward = glm::normalize(
                glm::vec3(
                        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                        sin(glm::radians(pitch)),
                        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
                )
        );
        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(right, forward));
    }

}