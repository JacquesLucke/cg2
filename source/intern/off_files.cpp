#include <iostream>
#include <fstream>
#include "../off_files.hpp"

OffFileData *readOffFile(std::string path) {
    OffFileData *data = new OffFileData();

    std::ifstream fs(path);

    std::string line;
    getline(fs, line);
    if (line != "OFF") {
        std::cerr << "file does not start with 'OFF" << std::endl;
        return nullptr;
    }

    getline(fs, line);
    int vertexAmount = std::stoi(line);
    for (int i = 0; i < vertexAmount; i++) {
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