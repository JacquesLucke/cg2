#include "window_controller.hpp"
#include "off_files.hpp"
#include "shader.hpp"

class KDTreeViewer : public WindowController {
public:
    KDTreeViewer(Window* window)
        : WindowController(window), buffer(0), color{1.0f, 1.0f, 1.0f, 1.0f}, offData(nullptr), shader(nullptr) {}

protected:
    bool onSetup() final override;
    //bool onTeardown() final override;
    //bool onUpdate() final override;
    void onRender() final override;
    void onRenderUI() final override;

private:
    unsigned int buffer;
    float color[4];
    OffFileData* offData;
    GLProgram* shader;
};