#pragma once

#include "mesh.hpp"

template<typename MeshType>
void smooth_UniformLaplacian(MeshType &mesh, float factor, int steps);

template<typename MeshType>
void smooth_UniformLaplacian_OneStep(MeshType &mesh, float factor);