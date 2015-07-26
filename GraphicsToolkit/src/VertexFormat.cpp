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


const int Vertex_PosTex::size = (int)sizeof(Vertex_PosTex);

const int Vertex_PosTex::posElements = (int)(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex_PosTex::posElemType = (unsigned)GL_FLOAT;
const void* Vertex_PosTex::posOffset = (void*)offsetof(Vertex_PosTex, position);

const int Vertex_PosTex::texCoordElements = (int)(sizeof(Vec2f) / sizeof(float));
const unsigned Vertex_PosTex::texCoordElemType = (unsigned)GL_FLOAT;
const void* Vertex_PosTex::texCoordOffset = (void*)offsetof(Vertex_PosTex, texCoord);