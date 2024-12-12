//
// Created by DELL on 2024/12/11.
//

#include <iostream>

#include <webgpu/webgpu_cpp.h>
#include <dawn/webgpu_cpp_print.h>

#include "common/Application.h"

int main()
{
    Application app;

    if (!app.Initialize()) {
        return 1;
    }

    // Warning: this is still not Emscripten-friendly, see below
    while (app.IsRunning()) {
        app.MainLoop();
    }

    app.Terminate();

    return 0;
}