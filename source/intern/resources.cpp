#include "../resources.hpp"

#include <fstream>
#include <sstream>

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

std::string loadRelTextFile(const std::string& path) {
    return loadTextFile(Resources::getPath(path));
}

OffFileData *loadRelOffFile(const std::string& path) {
    return loadOffFile(Resources::getPath(path));
}

GLProgram *loadRelShader(const std::string& path) {
    return loadShader(Resources::getPath(path));
}

std::string loadTextFile(const std::string& path) {
    std::ifstream fs(path);

    assert(fs.good());

    std::stringstream buffer;
    buffer << fs.rdbuf();
    fs.close();
    return buffer.str();
}

OffFileData *loadOffFile(const std::string& path) {
    std::ifstream fs(path);
    if (!fs.good())
    {
        std::cerr << "could not open file: " << path << std::endl;
        return nullptr;
    }

    std::string line;
    getline(fs, line);
    if (line != "OFF")
    {
        std::cerr << "file does not start with 'OFF" << std::endl;
        return nullptr;
    }

    OffFileData *data = new OffFileData();

    getline(fs, line);
    int vertexAmount = std::stoi(line);
    for (int i = 0; i < vertexAmount; i++)
    {
        getline(fs, line);
        int split1 = line.find(" ");
        int split2 = line.find(" ", split1 + 1);

        Vector<3> point;
        point[0] = std::stof(line.c_str() + 0);
        point[1] = std::stof(line.c_str() + split1);
        point[2] = std::stof(line.c_str() + split2);
        data->positions.push_back(point);
    }

    fs.close();

    return data;
}

GLProgram *loadShader(const std::string& path) {
    std::string source = loadTextFile(path);

    int vsStart = source.find("// Vertex Shader");
    int fsStart = source.find("// Fragment Shader");
    int vsLength = fsStart - vsStart;

    std::string vertexShader = source.substr(vsStart, vsLength);
    std::string fragmentShader = source.substr(fsStart);

    return new GLProgram(vertexShader, fragmentShader);
}
