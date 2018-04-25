#include <vector>
#include <string>
#include <vector.h>

struct OffFileData {
    std::vector<Vector<3>> positions;
};

OffFileData *readOffFile(std::string path);