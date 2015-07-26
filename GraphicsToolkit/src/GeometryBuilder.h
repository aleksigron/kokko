#pragma once

#include "App.h"
#include "Buffer.h"
#include "VertexFormat.h"

namespace GeometryBuilder
{
	RenderObjectId UnitCubeWithColor()
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
		index[1] = 1;
		index[2] = 3;
		
		index[3] = 3;
		index[4] = 2;
		index[5] = 0;
		
		// Front
		index[6] = 0;
		index[7] = 4;
		index[8] = 5;
		
		index[9] = 5;
		index[10] = 1;
		index[11] = 0;
		
		// Left
		index[12] = 0;
		index[13] = 2;
		index[14] = 6;
		
		index[15] = 6;
		index[16] = 4;
		index[17] = 0;
		
		// Back
		index[18] = 2;
		index[19] = 6;
		index[20] = 7;
		
		index[21] = 7;
		index[22] = 3;
		index[23] = 2;
		
		// Right
		index[24] = 3;
		index[25] = 7;
		index[26] = 5;
		
		index[27] = 5;
		index[28] = 1;
		index[29] = 3;
		
		// Top
		index[30] = 4;
		index[31] = 6;
		index[32] = 7;
		
		index[33] = 7;
		index[34] = 5;
		index[35] = 4;
		
		Renderer* r = App::GetRenderer();
		
		RenderObjectId id = r->AddRenderObject();
		RenderObject& obj = r->GetRenderObject(id);
		obj.UploadVertexData_PosCol(index, vertex);
		
		return id;
	}

	RenderObjectId UnitCubeWithTextureCoords()
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
		index[1] = 1;
		index[2] = 3;

		index[3] = 3;
		index[4] = 2;
		index[5] = 0;

		// Front
		index[6] = 0;
		index[7] = 4;
		index[8] = 5;

		index[9] = 5;
		index[10] = 1;
		index[11] = 0;

		// Left
		index[12] = 0;
		index[13] = 2;
		index[14] = 6;

		index[15] = 6;
		index[16] = 4;
		index[17] = 0;

		// Back
		index[18] = 2;
		index[19] = 6;
		index[20] = 7;

		index[21] = 7;
		index[22] = 3;
		index[23] = 2;

		// Right
		index[24] = 3;
		index[25] = 7;
		index[26] = 5;

		index[27] = 5;
		index[28] = 1;
		index[29] = 3;

		// Top
		index[30] = 4;
		index[31] = 6;
		index[32] = 7;

		index[33] = 7;
		index[34] = 5;
		index[35] = 4;

		Renderer* r = App::GetRenderer();

		RenderObjectId id = r->AddRenderObject();
		RenderObject& obj = r->GetRenderObject(id);
		obj.UploadVertexData_PosTex(index, vertex);

		return id;
	}
}