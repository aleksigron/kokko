#include "VertexFormat.hpp"

#include <OpenGL/gl3.h>

const int Vertex_PosNor::size = static_cast<int>(sizeof(Vertex_PosNor));

const int Vertex_PosNor::posElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosNor::posElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosNor::posOffset = reinterpret_cast<void*>(offsetof(Vertex_PosNor, position));

const int Vertex_PosNor::norElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosNor::norElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosNor::norOffset = reinterpret_cast<void*>(offsetof(Vertex_PosNor, normal));


const int Vertex_PosCol::size = static_cast<int>(sizeof(Vertex_PosCol));

const int Vertex_PosCol::posElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosCol::posElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosCol::posOffset = reinterpret_cast<void*>(offsetof(Vertex_PosCol, position));

const int Vertex_PosCol::colElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosCol::colElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosCol::colOffset = reinterpret_cast<void*>(offsetof(Vertex_PosCol, color));


const int Vertex_PosNorCol::size = static_cast<int>(sizeof(Vertex_PosNorCol));

const int Vertex_PosNorCol::posElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosNorCol::posElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosNorCol::posOffset = reinterpret_cast<void*>(offsetof(Vertex_PosNorCol, position));

const int Vertex_PosNorCol::norElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosNorCol::norElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosNorCol::norOffset = reinterpret_cast<void*>(offsetof(Vertex_PosNorCol, normal));

const int Vertex_PosNorCol::colElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosNorCol::colElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosNorCol::colOffset = reinterpret_cast<void*>(offsetof(Vertex_PosNorCol, color));


const int Vertex_PosTex::size = static_cast<int>(sizeof(Vertex_PosTex));

const int Vertex_PosTex::posElements = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosTex::posElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosTex::posOffset = reinterpret_cast<void*>(offsetof(Vertex_PosTex, position));

const int Vertex_PosTex::texCoordElements = static_cast<int>(sizeof(Vec2f) / sizeof(float));
const unsigned Vertex_PosTex::texCoordElemType = static_cast<unsigned>(GL_FLOAT);
const void* Vertex_PosTex::texCoordOffset = reinterpret_cast<void*>(offsetof(Vertex_PosTex, texCoord));