//
// Created by DELL on 2024/12/12.
//

#pragma once

#include <GLFW/glfw3.h>


class Application {
public:
    // Initialize everything and return true if it went all right
    bool Initialize();

    // Uninitialize everything that was initialized
    void Terminate();

    // Draw a frame and handle events
    void MainLoop();

    // Return true as long as the main loop should keep on running
    bool IsRunning();



private:

    GLFWwindow* window{};

};



