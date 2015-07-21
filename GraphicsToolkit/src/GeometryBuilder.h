#pragma once

#include "App.h"
#include "Buffer.h"

namespace GeometryBuilder
{
	void UnitCube()
	{
		Renderer* r = App::GetRenderer();
		
		RenderObject& obj = r->CreateRenderObject();
		
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
		
		obj.SetVertexBufferData(vertexData);
	}
}