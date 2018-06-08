#include "../resources.hpp"

#include <fstream>
#include <sstream>

#include <iostream>

namespace Resources
{
    static std::string subPath("");

    void init(int argc, char *argv[]) {
        std::string arg(argv[0]);
        std::string::size_type found = arg.find_last_of("/\\");

        subPath = (std::string::npos == found ? "" : arg.substr(0, found));
        subPath.append("/resources/");
    }

    std::string getPath(const std::string& path) {
        std::string res = subPath;
        res.append(path);

        return res;
    }
}

std::string loadRelTextResource(const std::string& path) {
    return loadTextResource(Resources::getPath(path));
}

OffFileData *loadRelOffResource(const std::string& path) {
    return loadOffResource(Resources::getPath(path));
}

GLProgram *loadRelShaderResource(const std::string& path) {
    return loadShaderResource(Resources::getPath(path));
}

std::string loadTextResource(const std::string& path) {
    std::ifstream fs(path);

    assert(fs.good());

    std::stringstream buffer;
    buffer << fs.rdbuf();
    fs.close();
    return buffer.str();
}

bool loadPosition(const std::string &line, OffFileData *data)
{
    static float x,y,z;

    int split1 = line.find(" ");
    int split2 = line.find(" ", split1 + 1);

    x = std::stof(line.c_str() + 0);
    y = std::stof(line.c_str() + split1);
    z = std::stof(line.c_str() + split2);

    data->positions.push_back(glm::vec3(x, y, z));

    return true;
}

bool loadIndices(const std::string &line, OffFileData *data)
{
    if(std::stoi(line) != 3) {
        std::cerr << "could not read indices because index count was unequal 3" << std::endl;
        return false;
    }

    int cur = 0;
    for(int i = 0; i < 3 /* index count per triangle */; i++) {
        cur = line.find(" ", cur+1);
        data->indices.push_back(std::stoi(line.c_str() + cur));
    }

    return true;
}

OffFileData *loadOffResource(const std::string& path) {
    std::ifstream fs(path);
    if (!fs.good()) {
        std::cerr << "could not open file: " << path << std::endl;
        return nullptr;
    }

    std::string line;
    getline(fs, line);
    if (line != "OFF") {
        std::cerr << "file does not start with 'OFF" << std::endl;
        return nullptr;
    }

    OffFileData *data = new OffFileData();

    // Get vertex and index count
    getline(fs, line);
    int vertexCount = std::stoi(line);
    int split = line.find(" ");
    int indexCount = std::stoi(line.c_str() + split);

    // Load positions
    for (int i = 0; i < vertexCount; i++) {
        getline(fs, line);
        if (!loadPosition(line, data)) {
            delete data;
            return nullptr;
        }
    }

    // Load indices
    for (int i = 0; i < indexCount; i++) {
        getline(fs, line);
        if (!loadIndices(line, data)) {
            delete data;
            return nullptr;
        }
    }

    fs.close();

    return data;
}

GLProgram *loadShaderResource(const std::string& path) {
    std::string source = loadTextResource(path);

    int vsStart = source.find("// Vertex Shader");
    int fsStart = source.find("// Fragment Shader");
    int vsLength = fsStart - vsStart;

    std::string vertexShader = source.substr(vsStart, vsLength);
    std::string fragmentShader = source.substr(fsStart);

    return new GLProgram(vertexShader, fragmentShader);
}
