#include <string>

class GLShader {
public:
    unsigned int shaderID;
    std::string source;

    GLShader(std::string source);

    virtual void compile() = 0;
    virtual void deleteCompiledResult();
};

class GLVertexShader : public GLShader {
public:
    GLVertexShader(std::string source) : GLShader(source) {};

    void compile() final override;
};

class GLFragmentShader : public GLShader {
public:
    GLFragmentShader(std::string source) : GLShader(source) {};

    void compile() final override;
};

class GLProgram {
public:
    unsigned int programID;
    GLVertexShader *vertexShader;
    GLFragmentShader *fragmentShader;

    GLProgram(std::string vertexShader, std::string fragmentShader);
    ~GLProgram();
    void compile();
};