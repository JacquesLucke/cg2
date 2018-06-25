#include "../resources.hpp"

#include <fstream>
#include <sstream>

#include <iostream>

namespace Resources
{
    static std::string subPath("");

    void init(int argc, char *argv[]) {
        std::string arg(argv[0]);
        std::cout << "arg: " << arg << std::endl;
        std::string::size_type found = arg.find_last_of("/\\");

        subPath = (std::string::npos == found ? "" : arg.substr(0, found+1));
        subPath.append("resources/");
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

NOffFileData *loadRelNOffResource(const std::string& path) {
    return loadNOffResource(Resources::getPath(path));
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

bool loadPosition(const std::string &line, OffFileData *data) {
    glm::vec3 p;
    if (sscanf(line.c_str(), "%f %f %f", &p.x, &p.y, &p.z) != 3) {
        return false;
    }
    data->positions.push_back(p);
    return true;
}

bool loadPositionAndNormal(const std::string& line, NOffFileData *data) {
    glm::vec3 p, n;
    if (sscanf(line.c_str(), "%f %f %f %f %f %f", &p.x, &p.y, &p.z, &n.x, &n.y, &n.z) != 6) {
        return false;
    }
    data->positions.push_back(p);
    data->normals.push_back(n);
    return true;
}

bool loadIndices(const std::string &line, std::vector<unsigned int> &indices) {
    int amount, v1, v2, v3;
    if (sscanf(line.c_str(), "%d %d %d %d", &amount, &v1, &v2, &v3) != 4) {
        return false;
    }

    if (amount != 3) {
        std::cerr << "could not read indices because index count was unequal 3" << std::endl;
        return false;
    }

    indices.push_back(v1);
    indices.push_back(v2);
    indices.push_back(v3);
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
        if (!loadIndices(line, data->indices)) {
            delete data;
            return nullptr;
        }
    }

    fs.close();

    return data;
}

NOffFileData *loadNOffResource(const std::string& path) {
    std::ifstream fs(path);
    if (!fs.good()) {
        std::cerr << "could not open file: " << path << std::endl;
        return nullptr;
    }

    std::string line;
    getline(fs, line);
    if (line != "NOFF") {
        std::cerr << "file does not start with 'NOFF" << std::endl;
        return nullptr;
    }

    NOffFileData *data = new NOffFileData();

    // Get vertex and index count
    getline(fs, line);
    int vertexCount = std::stoi(line);
    int split = line.find(" ");
    int indexCount = std::stoi(line.c_str() + split);

    // Load positions and normals
    for (int i = 0; i < vertexCount; i++) {
        getline(fs, line);
        if (!loadPositionAndNormal(line, data)) {
            delete data;
            return nullptr;
        }
    }

    // Load indices
    for (int i = 0; i < indexCount; i++) {
        getline(fs, line);
        if (!loadIndices(line, data->indices)) {
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
