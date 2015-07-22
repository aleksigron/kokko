#pragma once

#include "App.h"
#include "Buffer.h"

namespace GeometryBuilder
{
	RenderObjectId UnitCube()
	{
		Buffer<Vec3f> vertexData;
		vertexData.Allocate(36);
		
		// Bottom
		vertexData[0] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[1] = Vec3f(-0.5f, -0.5f, 0.5f);
		vertexData[2] = Vec3f(0.5f, -0.5f, 0.5f);
		
		vertexData[3] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[4] = Vec3f(0.5f, -0.5f, 0.5f);
		vertexData[5] = Vec3f(0.5f, -0.5f, -0.5f);
		
		// Front
		vertexData[6] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[7] = Vec3f(0.5f, 0.5f, -0.5f);
		vertexData[8] = Vec3f(-0.5f, 0.5f, -0.5f);
		
		vertexData[9] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[10] = Vec3f(0.5f, -0.5f, -0.5f);
		vertexData[11] = Vec3f(0.5f, 0.5f, -0.5f);
		
		// Left
		vertexData[12] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[13] = Vec3f(-0.5f, 0.5f, -0.5f);
		vertexData[14] = Vec3f(-0.5f, 0.5f, 0.5f);
		
		vertexData[15] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[16] = Vec3f(-0.5f, 0.5f, 0.5f);
		vertexData[17] = Vec3f(-0.5f, -0.5f, 0.5f);
		
		// Back
		vertexData[18] = Vec3f(-0.5f, -0.5f, 0.5f);
		vertexData[19] = Vec3f(-0.5f, 0.5f, 0.5f);
		vertexData[20] = Vec3f(0.5f, 0.5f, 0.5f);
		
		vertexData[21] = Vec3f(-0.5f, -0.5f, 0.5f);
		vertexData[22] = Vec3f(0.5f, 0.5f, 0.5f);
		vertexData[23] = Vec3f(0.5f, -0.5f, 0.5f);
		
		// Right
		vertexData[24] = Vec3f(0.5f, -0.5f, -0.5f);
		vertexData[25] = Vec3f(0.5f, 0.5f, 0.5f);
		vertexData[26] = Vec3f(0.5f, 0.5f, -0.5f);
		
		vertexData[27] = Vec3f(0.5f, -0.5f, -0.5f);
		vertexData[28] = Vec3f(0.5f, -0.5f, 0.5f);
		vertexData[29] = Vec3f(0.5f, 0.5f, 0.5f);
		
		// Top
		vertexData[30] = Vec3f(-0.5f, 0.5f, -0.5f);
		vertexData[31] = Vec3f(0.5f, 0.5f, 0.5f);
		vertexData[32] = Vec3f(-0.5f, 0.5f, 0.5f);
		
		vertexData[33] = Vec3f(-0.5f, 0.5f, -0.5f);
		vertexData[34] = Vec3f(0.5f, 0.5f, -0.5f);
		vertexData[35] = Vec3f(0.5f, 0.5f, 0.5f);
		
		Renderer* r = App::GetRenderer();
		
		RenderObjectId id = r->AddRenderObject();
		RenderObject& obj = r->GetRenderObject(id);
		r->UploadVertexPositionData(obj, vertexData);
		
		return id;
	}
	
	RenderObjectId UnitCubeWithColor()
	{
		Buffer<Vec3f> vertexData;
		vertexData.Allocate(36);
		
		// Bottom
		vertexData[0] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[1] = Vec3f(-0.5f, -0.5f, 0.5f);
		vertexData[2] = Vec3f(0.5f, -0.5f, 0.5f);
		
		vertexData[3] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[4] = Vec3f(0.5f, -0.5f, 0.5f);
		vertexData[5] = Vec3f(0.5f, -0.5f, -0.5f);
		
		// Front
		vertexData[6] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[7] = Vec3f(0.5f, 0.5f, -0.5f);
		vertexData[8] = Vec3f(-0.5f, 0.5f, -0.5f);
		
		vertexData[9] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[10] = Vec3f(0.5f, -0.5f, -0.5f);
		vertexData[11] = Vec3f(0.5f, 0.5f, -0.5f);
		
		// Left
		vertexData[12] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[13] = Vec3f(-0.5f, 0.5f, -0.5f);
		vertexData[14] = Vec3f(-0.5f, 0.5f, 0.5f);
		
		vertexData[15] = Vec3f(-0.5f, -0.5f, -0.5f);
		vertexData[16] = Vec3f(-0.5f, 0.5f, 0.5f);
		vertexData[17] = Vec3f(-0.5f, -0.5f, 0.5f);
		
		// Back
		vertexData[18] = Vec3f(-0.5f, -0.5f, 0.5f);
		vertexData[19] = Vec3f(-0.5f, 0.5f, 0.5f);
		vertexData[20] = Vec3f(0.5f, 0.5f, 0.5f);
		
		vertexData[21] = Vec3f(-0.5f, -0.5f, 0.5f);
		vertexData[22] = Vec3f(0.5f, 0.5f, 0.5f);
		vertexData[23] = Vec3f(0.5f, -0.5f, 0.5f);
		
		// Right
		vertexData[24] = Vec3f(0.5f, -0.5f, -0.5f);
		vertexData[25] = Vec3f(0.5f, 0.5f, 0.5f);
		vertexData[26] = Vec3f(0.5f, 0.5f, -0.5f);
		
		vertexData[27] = Vec3f(0.5f, -0.5f, -0.5f);
		vertexData[28] = Vec3f(0.5f, -0.5f, 0.5f);
		vertexData[29] = Vec3f(0.5f, 0.5f, 0.5f);
		
		// Top
		vertexData[30] = Vec3f(-0.5f, 0.5f, -0.5f);
		vertexData[31] = Vec3f(0.5f, 0.5f, 0.5f);
		vertexData[32] = Vec3f(-0.5f, 0.5f, 0.5f);
		
		vertexData[33] = Vec3f(-0.5f, 0.5f, -0.5f);
		vertexData[34] = Vec3f(0.5f, 0.5f, -0.5f);
		vertexData[35] = Vec3f(0.5f, 0.5f, 0.5f);
		
		Buffer<Vec3f> colorData;
		colorData.Allocate(36);
		
		// Bottom
		colorData[0] = Vec3f(0.0f, 0.0f, 0.0f);
		colorData[1] = Vec3f(0.0f, 0.0f, 1.0f);
		colorData[2] = Vec3f(1.0f, 0.0f, 1.0f);
		
		colorData[3] = Vec3f(0.0f, 0.0f, 0.0f);
		colorData[4] = Vec3f(1.0f, 0.0f, 1.0f);
		colorData[5] = Vec3f(1.0f, 0.0f, 0.0f);
		
		// Front
		colorData[6] = Vec3f(0.0f, 0.0f, 0.0f);
		colorData[7] = Vec3f(1.0f, 1.0f, 0.0f);
		colorData[8] = Vec3f(0.0f, 1.0f, 0.0f);
		
		colorData[9] = Vec3f(0.0f, 0.0f, 0.0f);
		colorData[10] = Vec3f(1.0f, 0.0f, 0.0f);
		colorData[11] = Vec3f(1.0f, 1.0f, 0.0f);
		
		// Left
		colorData[12] = Vec3f(0.0f, 0.0f, 0.0f);
		colorData[13] = Vec3f(0.0f, 1.0f, 0.0f);
		colorData[14] = Vec3f(0.0f, 1.0f, 1.0f);
		
		colorData[15] = Vec3f(0.0f, 0.0f, 0.0f);
		colorData[16] = Vec3f(0.0f, 1.0f, 1.0f);
		colorData[17] = Vec3f(0.0f, 0.0f, 1.0f);
		
		// Back
		colorData[18] = Vec3f(0.0f, 0.0f, 1.0f);
		colorData[19] = Vec3f(0.0f, 1.0f, 1.0f);
		colorData[20] = Vec3f(1.0f, 1.0f, 1.0f);
		
		colorData[21] = Vec3f(0.0f, 0.0f, 1.0f);
		colorData[22] = Vec3f(1.0f, 1.0f, 1.0f);
		colorData[23] = Vec3f(1.0f, 0.0f, 1.0f);
		
		// Right
		colorData[24] = Vec3f(1.0f, 0.0f, 0.0f);
		colorData[25] = Vec3f(1.0f, 1.0f, 1.0f);
		colorData[26] = Vec3f(1.0f, 1.0f, 0.0f);
		
		colorData[27] = Vec3f(1.0f, 0.0f, 0.0f);
		colorData[28] = Vec3f(1.0f, 0.0f, 1.0f);
		colorData[29] = Vec3f(1.0f, 1.0f, 1.0f);
		
		// Top
		colorData[30] = Vec3f(0.0f, 1.0f, 0.0f);
		colorData[31] = Vec3f(1.0f, 1.0f, 1.0f);
		colorData[32] = Vec3f(0.0f, 1.0f, 1.0f);
		
		colorData[33] = Vec3f(0.0f, 1.0f, 0.0f);
		colorData[34] = Vec3f(1.0f, 1.0f, 0.0f);
		colorData[35] = Vec3f(1.0f, 1.0f, 1.0f);
		
		Renderer* r = App::GetRenderer();
		
		RenderObjectId id = r->AddRenderObject();
		RenderObject& obj = r->GetRenderObject(id);
		r->UploadVertexPositionData(obj, vertexData);
		r->UploadVertexColorData(obj, colorData);
		
		return id;
	}
}