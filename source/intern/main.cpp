#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <iostream>
#include <string>

#include "../app.hpp"
#include "../vector.hpp"
#include "../kdtree.hpp"

#define NDIM 3

template<int N>
using VectorKDTree = KDTree<Vector<N>, N, getVectorIndex<N>, Vector<N>::distance>;

using namespace cgX;

App *createApp(const Config& config) {
    App *app = new TestApp();

    if(!app->setup("CG 2", config)) {
        std::cout << "App setup failed" << std::endl;
        delete app;
        return nullptr;
    }
    return app;
}

int main(void) {
    Config config(640, 480);

    App *app = createApp(config);

    if(nullptr == app)
        return 1;

    while(app->running()) {
        app->update();
        app->render();
    }

    app->teardown();
    delete app;

    return 0;
}
