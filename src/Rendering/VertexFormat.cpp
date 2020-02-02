#include "Rendering/VertexFormat.hpp"

#include "System/IncludeOpenGL.hpp"

// Generic: 3 float

const int Vertex3f::size = static_cast<int>(sizeof(Vertex3f));

const int Vertex3f::aElemCount = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex3f::aElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f::aOffset = offsetof(Vertex3f, a);

// Generic: 4 float

const int Vertex4f::size = static_cast<int>(sizeof(Vertex4f));

const int Vertex4f::aElemCount = static_cast<int>(sizeof(Vec4f) / sizeof(float));
const unsigned Vertex4f::aElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex4f::aOffset = offsetof(Vertex4f, a);

// Generic: 3 float, 2 float

const int Vertex3f2f::size = static_cast<int>(sizeof(Vertex3f2f));

const int Vertex3f2f::aElemCount = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex3f2f::aElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f2f::aOffset = offsetof(Vertex3f2f, a);

const int Vertex3f2f::bElemCount = static_cast<int>(sizeof(Vec2f) / sizeof(float));
const unsigned Vertex3f2f::bElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f2f::bOffset = offsetof(Vertex3f2f, b);

// Generic: 3 float, 3 float

const int Vertex3f3f::size = static_cast<int>(sizeof(Vertex3f3f));

const int Vertex3f3f::aElemCount = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex3f3f::aElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f3f::aOffset = offsetof(Vertex3f3f, a);

const int Vertex3f3f::bElemCount = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex3f3f::bElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f3f::bOffset = offsetof(Vertex3f3f, b);

// Generic: 3 float, 3 float, 2 float

const int Vertex3f3f2f::size = static_cast<int>(sizeof(Vertex3f3f2f));

const int Vertex3f3f2f::aElemCount = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex3f3f2f::aElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f3f2f::aOffset = offsetof(Vertex3f3f2f, a);

const int Vertex3f3f2f::bElemCount = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex3f3f2f::bElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f3f2f::bOffset = offsetof(Vertex3f3f2f, b);

const int Vertex3f3f2f::cElemCount = static_cast<int>(sizeof(Vec2f) / sizeof(float));
const unsigned Vertex3f3f2f::cElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f3f2f::cOffset = offsetof(Vertex3f3f2f, c);

// Generic: 3 float, 3 float, 3 float

const int Vertex3f3f3f::size = static_cast<int>(sizeof(Vertex3f3f3f));

const int Vertex3f3f3f::aElemCount = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex3f3f3f::aElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f3f3f::aOffset = offsetof(Vertex3f3f3f, a);

const int Vertex3f3f3f::bElemCount = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex3f3f3f::bElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f3f3f::bOffset = offsetof(Vertex3f3f3f, b);

const int Vertex3f3f3f::cElemCount = static_cast<int>(sizeof(Vec3f) / sizeof(float));
const unsigned Vertex3f3f3f::cElemType = static_cast<unsigned>(GL_FLOAT);
const std::size_t Vertex3f3f3f::cOffset = offsetof(Vertex3f3f3f, c);
