#include <glm/glm.hpp>

#include "window_controller.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"

class KDTreeViewer : public WindowController {
public:
    KDTreeViewer(Window* window)
        : WindowController(window),
          buffer(0), color{1.0f, 1.0f, 1.0f, 1.0f}, mesh(nullptr), shader(nullptr),
          camera(new CameraController(new PerspectiveCamera(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), 1, window->aspect()), window)) {}

protected:
    bool onSetup() final override;
    //void onTeardown() final override;
    void onUpdate() final override;
    void onRender() final override;
    void onRenderUI() final override;

private:
    bool isKeyDown(int key);

    unsigned int buffer;
    float color[4];
    Mesh<VertexP>* mesh;
    GLProgram* shader;
    CameraController* camera;
};
