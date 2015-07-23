#include "VertexFormat.h"

#include <OpenGL/gltypes.h>
#include <OpenGL/gl3.h>

const int Vertex_PosCol::size = (int)sizeof(Vertex_PosCol);

const int Vertex_PosCol::posElements = (int)(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosCol::posElemType = (unsigned)GL_FLOAT;
const void* Vertex_PosCol::posOffset = (void*)offsetof(Vertex_PosCol, position);

const int Vertex_PosCol::colElements = (int)(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosCol::colElemType = (unsigned)GL_FLOAT;
const void* Vertex_PosCol::colOffset = (void*)offsetof(Vertex_PosCol, color);