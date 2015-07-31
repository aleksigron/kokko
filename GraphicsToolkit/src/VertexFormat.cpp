#include "VertexFormat.h"

#include <OpenGL/gl3.h>

const int Vertex_PosCol::size = static_cast<int>(sizeof(Vertex_PosCol));

const int Vertex_PosCol::posElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosCol::posElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosCol::posOffset = reinterpret_cast<void*>(offsetof(Vertex_PosCol, position));

const int Vertex_PosCol::colElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosCol::colElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosCol::colOffset = reinterpret_cast<void*>(offsetof(Vertex_PosCol, color));


const int Vertex_PosTex::size = static_cast<int>(sizeof(Vertex_PosTex));

const int Vertex_PosTex::posElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosTex::posElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosTex::posOffset = reinterpret_cast<void*>(offsetof(Vertex_PosTex, position));

const int Vertex_PosTex::texCoordElements = static_cast<int>(sizeof(Vec2f) / sizeof(float));
const unsigned Vertex_PosTex::texCoordElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosTex::texCoordOffset = reinterpret_cast<void*>(offsetof(Vertex_PosTex, texCoord));