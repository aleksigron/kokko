#include "DebugTextRenderer.hpp"

#include <cstring>
#include <cassert>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "File.hpp"
#include "BitmapFont.hpp"
#include "VertexFormat.hpp"

#include "Mesh.hpp"
#include "Material.hpp"
#include "ResourceManager.hpp"
#include "App.hpp"

DebugTextRenderer::DebugTextRenderer()
{
	font = nullptr;

	renderDataCount = 0;
	renderDataAllocated = 32;
	renderData = new RenderData[renderDataAllocated];

	stringDataUsed = 0;
	stringDataAllocated = 4096;
	stringData = new char[renderDataAllocated];
}

DebugTextRenderer::~DebugTextRenderer()
{
	delete font;
	delete[] renderData;
	delete[] stringData;
}

bool DebugTextRenderer::LoadBitmapFont(const char* filePath)
{
	Buffer<char> content = File::ReadText(filePath);

	if (content.IsValid())
	{
		font = new BitmapFont;
		return font->LoadFromBDF(content);
	}
	else
		return false;
}

void DebugTextRenderer::AddText(StringRef str, Vec2f position)
{
	if (renderDataCount < renderDataAllocated)
	{
		if (stringDataUsed + str.len <= stringDataAllocated)
		{
			RenderData* rd = renderData + renderDataCount;

			// Copy string data

			char* stringLocation = stringData + stringDataUsed;
			rd->string.str = stringLocation;
			rd->string.len = str.len;

			stringDataUsed += str.len;
			std::memcpy(stringLocation, str.str, str.len);

			// Set draw area

			rd->area = Rectangle(position, Vec2f(1024.0f, 1024.0f));

			++renderDataCount;
		}
	}
}

void DebugTextRenderer::Render()
{
	if (renderDataCount > 0)
	{
		Mesh mesh;
		this->CreateAndUploadData(mesh);

		ResourceManager* rm = App::GetResourceManager();
		Shader* shader = rm->GetShader("res/shaders/debug_text.shader.json");

		const ShaderUniform* u = nullptr;
		for (unsigned int i = 0; i < shader->materialUniformCount; ++i)
		{
			if (shader->materialUniforms[i].type == ShaderUniformType::Tex2D)
			{
				u = shader->materialUniforms + i;
			}
		}

		glUseProgram(shader->driverId);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, font->GetTextureDriverId());
		glUniform1i(u->location, 0);

		glBindVertexArray(mesh.vertexArrayObject);

		glDrawElements(GL_TRIANGLES, mesh.indexCount, mesh.indexElementType, nullptr);

		mesh.DeleteBuffers();

		renderDataCount = 0;
		stringDataUsed = 0;
	}
}

void DebugTextRenderer::CreateAndUploadData(Mesh& mesh)
{
	assert(stringDataUsed * 4 < (1 << 16));

	Buffer<Vertex_PosTex> vertexBuffer;
	vertexBuffer.Allocate(stringDataUsed * 4);

	Buffer<unsigned short> indexBuffer;
	indexBuffer.Allocate(stringDataUsed * 6);

	const BitmapGlyph* glyphBegin = font->GlyphsBegin();
	const BitmapGlyph* glyphEnd = font->GlyphsEnd();

	Vec2f textureSize = font->GetTextureSize();

	Vertex_PosTex* vertexBegin = vertexBuffer.Data();
	Vertex_PosTex* vertexItr = vertexBegin;
	unsigned short* indexItr = indexBuffer.Data();

	RenderData* rdItr = renderData;
	RenderData* rdEnd = rdItr + renderDataCount;
	for (; rdItr != rdEnd; ++rdItr)
	{
		RenderData rd = *rdItr;
		Vec2f drawPos = rd.area.position;

		const char* strItr = rd.string.str;
		const char* strEnd = strItr + rd.string.len;
		for (; strItr != strEnd; ++strItr)
		{
			uint32_t c = *strItr;

			for (const BitmapGlyph* gItr = glyphBegin; gItr != glyphEnd; ++gItr)
			{
				if (gItr->codePoint == c)
				{
					const BitmapGlyph& glyph = *gItr;

					Vertex_PosTex& v00 = vertexItr[0];
					Vertex_PosTex& v10 = vertexItr[1];
					Vertex_PosTex& v01 = vertexItr[2];
					Vertex_PosTex& v11 = vertexItr[3];

					Vec2f quadPos = drawPos + glyph.drawOffset;
					float quadRight = glyph.size.x;
					float quadDown = -glyph.size.y;

					Vec2f uvPos;
					uvPos.x = glyph.texturePosition.x / textureSize.x;
					uvPos.y = glyph.texturePosition.y / textureSize.y;
					float uvRight = glyph.size.x / textureSize.x;
					float uvDown = glyph.size.y / textureSize.y;

					drawPos = drawPos + glyph.advance;

					v00.position.x = quadPos.x / frameSize.x;
					v00.position.y = quadPos.y / frameSize.y;
					v00.position.z = 0.0f;
					v00.texCoord.x = uvPos.x;
					v00.texCoord.y = uvPos.y;

					v10.position.x = (quadPos.x + quadRight) / frameSize.x;
					v10.position.y = quadPos.y / frameSize.y;
					v10.position.z = 0.0f;
					v10.texCoord.x = uvPos.x + uvRight;
					v10.texCoord.y = uvPos.y;

					v01.position.x = quadPos.x / frameSize.x;
					v01.position.y = (quadPos.y + quadDown) / frameSize.y;
					v01.position.z = 0.0f;
					v01.texCoord.x = uvPos.x;
					v01.texCoord.y = uvPos.y + uvDown;

					v11.position.x = (quadPos.x + quadRight) / frameSize.x;
					v11.position.y = (quadPos.y + quadDown) / frameSize.y;
					v11.position.z = 0.0f;
					v11.texCoord.x = uvPos.x + uvRight;
					v11.texCoord.y = uvPos.y + uvDown;

					unsigned short idx = static_cast<unsigned short>(vertexItr - vertexBegin);

					indexItr[0] = idx + 0;
					indexItr[1] = idx + 3;
					indexItr[2] = idx + 1;
					indexItr[3] = idx + 0;
					indexItr[4] = idx + 2;
					indexItr[5] = idx + 3;

					break;
				}
			}

			vertexItr += 4;
			indexItr += 6;
		}
	}

	float* vb = reinterpret_cast<float*>(vertexBuffer.Data());
	unsigned int vbs = static_cast<unsigned int>(vertexBuffer.Count());
	unsigned short* ib = indexBuffer.Data();
	unsigned int ibs = static_cast<unsigned int>(indexBuffer.Count());

	mesh.Upload_PosTex(vb, vbs, ib, ibs);
}
