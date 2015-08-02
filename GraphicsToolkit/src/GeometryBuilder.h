#pragma once

#include "App.h"
#include "Buffer.h"
#include "VertexFormat.h"

namespace GeometryBuilder
{
	ObjectId UnitCubeWithColor()
	{
		Buffer<Vertex_PosCol> vertex;
		vertex.Allocate(8);
		
		vertex[0] = Vertex_PosCol { Vec3f(-0.5f, -0.5f, -0.5f), Vec3f(0.0f, 0.0f, 0.0f) };
		vertex[1] = Vertex_PosCol { Vec3f(0.5f, -0.5f, -0.5f), Vec3f(1.0f, 0.0f, 0.0f) };
		vertex[2] = Vertex_PosCol { Vec3f(-0.5f, -0.5f, 0.5f), Vec3f(0.0f, 0.0f, 1.0f) };
		vertex[3] = Vertex_PosCol { Vec3f(0.5f, -0.5f, 0.5f), Vec3f(1.0f, 0.0f, 1.0f) };
		vertex[4] = Vertex_PosCol { Vec3f(-0.5f, 0.5f, -0.5f), Vec3f(0.0f, 1.0f, 0.0f) };
		vertex[5] = Vertex_PosCol { Vec3f(0.5f, 0.5f, -0.5f), Vec3f(1.0f, 1.0f, 0.0f) };
		vertex[6] = Vertex_PosCol { Vec3f(-0.5f, 0.5f, 0.5f), Vec3f(0.0f, 1.0f, 1.0f) };
		vertex[7] = Vertex_PosCol { Vec3f(0.5f, 0.5f, 0.5f), Vec3f(1.0f, 1.0f, 1.0f) };
		
		Buffer<uint16_t> index;
		index.Allocate(36);
		
		// Bottom
		index[0] = 0;
		index[1] = 3;
		index[2] = 2;

		index[3] = 0;
		index[4] = 1;
		index[5] = 3;

		// Front
		index[6] = 2;
		index[7] = 7;
		index[8] = 6;

		index[9] = 2;
		index[10] = 3;
		index[11] = 7;

		// Left
		index[12] = 2;
		index[13] = 6;
		index[14] = 4;

		index[15] = 2;
		index[16] = 4;
		index[17] = 0;

		// Back
		index[18] = 0;
		index[19] = 4;
		index[20] = 5;

		index[21] = 0;
		index[22] = 5;
		index[23] = 1;

		// Right
		index[24] = 1;
		index[25] = 5;
		index[26] = 7;

		index[27] = 1;
		index[28] = 7;
		index[29] = 3;

		// Top
		index[30] = 4;
		index[31] = 6;
		index[32] = 7;

		index[33] = 4;
		index[34] = 7;
		index[35] = 5;
		
		Renderer* r = App::GetRenderer();
		
		ObjectId id = r->AddRenderObject();
		RenderObject& obj = r->GetRenderObject(id);
		obj.UploadVertexData_PosCol(index, vertex);
		
		return id;
	}

	ObjectId UnitCubeWithTextureCoords()
	{
		Buffer<Vertex_PosTex> vertex;
		vertex.Allocate(8);

		vertex[0] = Vertex_PosTex { Vec3f(-0.5f, -0.5f, -0.5f), Vec2f(0.0f, 0.0f) };
		vertex[1] = Vertex_PosTex { Vec3f(0.5f, -0.5f, -0.5f), Vec2f(1.0f, 0.0f) };
		vertex[2] = Vertex_PosTex { Vec3f(-0.5f, -0.5f, 0.5f), Vec2f(0.0f, 0.0f) };
		vertex[3] = Vertex_PosTex { Vec3f(0.5f, -0.5f, 0.5f), Vec2f(1.0f, 0.0f) };
		vertex[4] = Vertex_PosTex { Vec3f(-0.5f, 0.5f, -0.5f), Vec2f(0.0f, 1.0f) };
		vertex[5] = Vertex_PosTex { Vec3f(0.5f, 0.5f, -0.5f), Vec2f(1.0f, 1.0f) };
		vertex[6] = Vertex_PosTex { Vec3f(-0.5f, 0.5f, 0.5f), Vec2f(0.0f, 1.0f) };
		vertex[7] = Vertex_PosTex { Vec3f(0.5f, 0.5f, 0.5f), Vec2f(1.0f, 1.0f) };

		Buffer<uint16_t> index;
		index.Allocate(36);

		// Bottom
		index[0] = 0;
		index[1] = 3;
		index[2] = 2;

		index[3] = 0;
		index[4] = 1;
		index[5] = 3;

		// Front
		index[6] = 2;
		index[7] = 7;
		index[8] = 6;

		index[9] = 2;
		index[10] = 3;
		index[11] = 7;

		// Left
		index[12] = 2;
		index[13] = 6;
		index[14] = 4;

		index[15] = 2;
		index[16] = 4;
		index[17] = 0;

		// Back
		index[18] = 0;
		index[19] = 4;
		index[20] = 5;

		index[21] = 0;
		index[22] = 5;
		index[23] = 1;

		// Right
		index[24] = 1;
		index[25] = 5;
		index[26] = 7;

		index[27] = 1;
		index[28] = 7;
		index[29] = 3;

		// Top
		index[30] = 4;
		index[31] = 6;
		index[32] = 7;

		index[33] = 4;
		index[34] = 7;
		index[35] = 5;

		Renderer* r = App::GetRenderer();

		ObjectId id = r->AddRenderObject();
		RenderObject& obj = r->GetRenderObject(id);
		obj.UploadVertexData_PosTex(index, vertex);

		return id;
	}
}