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

DebugTextRenderer::DebugTextRenderer() :
	scaleFactor(1.0f)
{
	font = nullptr;

	renderDataCount = 0;
	renderDataAllocated = 32;
	renderData = new RenderData[renderDataAllocated];

	stringDataUsed = 0;
	stringDataAllocated = 4096;
	stringData = new char[stringDataAllocated];
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

			rd->area.position = position;
			rd->area.size = scaledFrameSize - position;

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

		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	Buffer<Vertex3f2f> vertexBuffer;
	vertexBuffer.Allocate(stringDataUsed * 4);

	Buffer<unsigned short> indexBuffer;
	indexBuffer.Allocate(stringDataUsed * 6);

	const BitmapGlyph* glyphBegin = font->GlyphsBegin();
	const BitmapGlyph* glyphEnd = font->GlyphsEnd();

	int fontLineHeight = font->GetLineHeight();
	Vec2f textureSize = font->GetTextureSize();
	Vec2f invTexSize = Vec2f(1.0f / textureSize.x, 1.0f / textureSize.y);

	Vec2f scaledInvFrame = Vec2f(2.0f / scaledFrameSize.x, 2.0f / scaledFrameSize.y);

	Vertex3f2f* vertexBegin = vertexBuffer.Data();
	Vertex3f2f* vertexItr = vertexBegin;
	unsigned short* indexItr = indexBuffer.Data();

	RenderData* rdItr = renderData;
	RenderData* rdEnd = rdItr + renderDataCount;
	for (; rdItr != rdEnd; ++rdItr)
	{
		RenderData rd = *rdItr;
		Vec2f drawPos = Vec2f(rd.area.position.x, -rd.area.position.y);

		const char* strItr = rd.string.str;
		const char* strEnd = strItr + rd.string.len;
		for (; strItr != strEnd; ++strItr)
		{
			uint32_t c = *strItr;
			unsigned short vertexIndex = static_cast<unsigned short>(vertexItr - vertexBegin);

			for (const BitmapGlyph* gItr = glyphBegin; gItr != glyphEnd; ++gItr)
			{
				if (gItr->codePoint == c)
				{
					const BitmapGlyph& glyph = *gItr;

					Vec2f quadPos = drawPos + glyph.drawOffset;
					float quadRight = glyph.size.x;
					float quadDown = -glyph.size.y;

					float uvX = glyph.texturePosition.x * invTexSize.x;
					float uvY = glyph.texturePosition.y * invTexSize.y;
					float uvRight = glyph.size.x * invTexSize.x;
					float uvDown = glyph.size.y * invTexSize.y;

					Vertex3f2f& v00 = vertexItr[0];
					v00.a.x = quadPos.x * scaledInvFrame.x - 1.0f;
					v00.a.y = quadPos.y * scaledInvFrame.y + 1.0f;
					v00.a.z = 0.0f;
					v00.b.x = uvX;
					v00.b.y = uvY;

					Vertex3f2f& v10 = vertexItr[1];
					v10.a.x = (quadPos.x + quadRight) * scaledInvFrame.x - 1.0f;
					v10.a.y = quadPos.y * scaledInvFrame.y + 1.0f;
					v10.a.z = 0.0f;
					v10.b.x = uvX + uvRight;
					v10.b.y = uvY;

					Vertex3f2f& v01 = vertexItr[2];
					v01.a.x = quadPos.x * scaledInvFrame.x - 1.0f;
					v01.a.y = (quadPos.y + quadDown) * scaledInvFrame.y + 1.0f;
					v01.a.z = 0.0f;
					v01.b.x = uvX;
					v01.b.y = uvY + uvDown;

					Vertex3f2f& v11 = vertexItr[3];
					v11.a.x = (quadPos.x + quadRight) * scaledInvFrame.x - 1.0f;
					v11.a.y = (quadPos.y + quadDown) * scaledInvFrame.y + 1.0f;
					v11.a.z = 0.0f;
					v11.b.x = uvX + uvRight;
					v11.b.y = uvY + uvDown;

					indexItr[0] = vertexIndex + 0;
					indexItr[1] = vertexIndex + 3;
					indexItr[2] = vertexIndex + 1;
					indexItr[3] = vertexIndex + 0;
					indexItr[4] = vertexIndex + 2;
					indexItr[5] = vertexIndex + 3;

					drawPos = drawPos + glyph.advance;
					if (drawPos.x >= scaledFrameSize.x)
					{
						drawPos.x = rd.area.position.x;
						drawPos.y = drawPos.y - fontLineHeight;
					}

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

	mesh.Upload_3f2f(vb, vbs, ib, ibs);
}
