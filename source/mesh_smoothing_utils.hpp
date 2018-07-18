#pragma once

#include "mesh.hpp"

template<typename MeshType>
void smooth_UniformLaplacian(MeshType &mesh, float factor, int steps);

template<typename MeshType>
void smooth_CotanLaplacian(MeshType &mesh, float factor, int steps, bool doImplicit = false);

template<typename MeshType>
void smooth_UniformLaplacian_OneStep(MeshType &mesh, float factor);

template<typename MeshType>
void smooth_CotanLaplacian_OneStep(MeshType &mesh, float factor, bool doImplicit);