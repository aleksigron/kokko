#include "Debug/DebugTextRenderer.hpp"

#include <cstring>
#include <cassert>

#include "IncludeOpenGL.hpp"

#include "File.hpp"
#include "BitmapFont.hpp"
#include "VertexFormat.hpp"
#include "EncodingUtf8.hpp"

#include "App.hpp"

#include "Engine.hpp"
#include "Shader.hpp"
#include "MeshManager.hpp"
#include "ResourceManager.hpp"

DebugTextRenderer::DebugTextRenderer() :
	font(nullptr),
	stringCharCount(0),
	scaleFactor(1.0f),
	meshId(MeshId{})
{
}

DebugTextRenderer::~DebugTextRenderer()
{
	delete font;
}

void DebugTextRenderer::SetFrameSize(const Vec2f& size)
{
	assert(renderData.GetCount() == 0);

	frameSize = size;
	scaledFrameSize = size * (1.0f / scaleFactor);
}

void DebugTextRenderer::SetScaleFactor(float scale)
{
	assert(renderData.GetCount() == 0);

	scaleFactor = scale;
	scaledFrameSize = frameSize * (1.0f / scaleFactor);
}

int DebugTextRenderer::GetRowCountForTextLength(unsigned int characterCount) const
{
	int glyphWidth = font->GetGlyphWidth();
	int screenWidth = static_cast<int>(scaledFrameSize.x);
	int charsPerRow = screenWidth / glyphWidth;

	return (characterCount + charsPerRow - 1) / charsPerRow;
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
	Rectanglef area(position, scaledFrameSize - position);

	this->AddText(str, area);
}

void DebugTextRenderer::AddText(StringRef str, const Rectanglef& area)
{
	stringCharCount += EncodingUtf8::CountCharacters(str);

	unsigned int stringPosition = stringData.GetCount();
	stringData.InsertBack(str.str, str.len);

	RenderData& rd = renderData.PushBack();
	rd.stringStart = stringPosition;
	rd.stringLength = str.len;
	rd.area = area;
}

void DebugTextRenderer::Render()
{
	if (renderData.GetCount() > 0 && font != nullptr)
	{
		Engine* engine = Engine::GetInstance();
		MeshManager* meshManager = engine->GetMeshManager();
		ResourceManager* rm = engine->GetResourceManager();

		if (meshId.IsValid() == false)
		{
			meshId = meshManager->CreateMesh();
		}

		CreateAndUploadData();

		Shader* shader = rm->GetShader("res/shaders/debug_text.shader.json");

		const ShaderUniform* textureUniform = nullptr;
		const ShaderUniform* shadowOffsetUniform = nullptr;
		for (unsigned int i = 0; i < shader->materialUniformCount; ++i)
		{
			ShaderUniformType type = shader->materialUniforms[i].type;

			if (type == ShaderUniformType::Tex2D)
			{
				textureUniform = shader->materialUniforms + i;
			}
			else if (type == ShaderUniformType::Float)
			{
				shadowOffsetUniform = shader->materialUniforms + i;
			}
		}

		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Use shader
		glUseProgram(shader->driverId);

		// Bind shadow offset
		if (shadowOffsetUniform != nullptr)
		{
			Vec2f texSize = font->GetTextureSize();
			glUniform1f(shadowOffsetUniform->location, 1.0f / texSize.y);
		}

		// Bind texture
		if (textureUniform != nullptr)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, font->GetTextureDriverId());
			glUniform1i(textureUniform->location, 0);
		}

		MeshDrawData* draw = meshManager->GetDrawData(meshId);

		// Bind vertex array object
		glBindVertexArray(draw->vertexArrayObject);

		// Draw
		glDrawElements(draw->primitiveMode, draw->indexCount, draw->indexElementType, nullptr);

		stringCharCount = 0;
		stringData.Clear();
		renderData.Clear();
	}
}

void DebugTextRenderer::CreateAndUploadData()
{
	// Make sure vertex indices fit in unsigned short type
	assert(stringCharCount * 4 < (1 << 16));

	vertexData.Resize(stringCharCount * 4);
	indexData.Resize(stringCharCount * 6);

	int fontLineHeight = font->GetLineHeight();
	Vec2f textureSize = font->GetTextureSize();
	Vec2f invTexSize = Vec2f(1.0f / textureSize.x, 1.0f / textureSize.y);

	Vec2f scaledInvFrame = Vec2f(2.0f / scaledFrameSize.x, 2.0f / scaledFrameSize.y);

	Vertex3f2f* vertexBegin = vertexData.GetData();
	Vertex3f2f* vertexItr = vertexBegin;
	unsigned short* indexItr = indexData.GetData();

	const char* strData = stringData.GetData();

	RenderData* rdItr = renderData.GetData();
	RenderData* rdEnd = rdItr + renderData.GetCount();
	for (; rdItr != rdEnd; ++rdItr)
	{
		RenderData rd = *rdItr;
		Vec2f drawPos = Vec2f(rd.area.position.x, -rd.area.position.y);

		const char* strItr = strData + rd.stringStart;
		const char* strEnd = strItr + rd.stringLength;
		while (strItr < strEnd)
		{
			unsigned int codepoint;
			unsigned int bytesDecoded = EncodingUtf8::DecodeCodepoint(strItr, codepoint);

			if (bytesDecoded > 0)
			{
				strItr += bytesDecoded;

				const BitmapGlyph* foundGlyph = font->GetGlyph(codepoint);

				if (foundGlyph != nullptr)
				{
					BitmapGlyph glyph = *foundGlyph;

					Vec2f quadPos = drawPos;
					float quadRight = glyph.size.x;
					float quadDown = -(glyph.size.y + 1.0f);

					float uvX = glyph.texturePosition.x * invTexSize.x;
					float uvY = glyph.texturePosition.y * invTexSize.y;
					float uvRight = glyph.size.x * invTexSize.x;
					float uvDown = glyph.size.y * invTexSize.y + invTexSize.y;

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

					unsigned short vertIndex = static_cast<unsigned short>(vertexItr - vertexBegin);

					indexItr[0] = vertIndex + 0;
					indexItr[1] = vertIndex + 3;
					indexItr[2] = vertIndex + 1;
					indexItr[3] = vertIndex + 0;
					indexItr[4] = vertIndex + 2;
					indexItr[5] = vertIndex + 3;

					drawPos.x += glyph.size.x;

					if (drawPos.x >= scaledFrameSize.x)
					{
						drawPos.x = rd.area.position.x;
						drawPos.y = drawPos.y - fontLineHeight;
					}
				}

				// TODO: Do we need to have empty char drawing in an else here?

				vertexItr += 4;
				indexItr += 6;
			}
			else // Not a valid UTF8 character, try next byte
			{
				strItr += 1;
			}
		}
	}

	IndexedVertexData<Vertex3f2f, unsigned short> data;
	data.primitiveMode = MeshPrimitiveMode::Triangles;
	data.vertData = vertexData.GetData();
	data.vertCount = vertexData.GetCount();
	data.idxData = indexData.GetData();
	data.idxCount = indexData.GetCount();

	Engine::GetInstance()->GetMeshManager()->Upload_3f2f(meshId, data);
}
