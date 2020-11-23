#include "Debug/DebugTextRenderer.hpp"

#include <cstring>
#include <cassert>

#include "Application/App.hpp"

#include "Core/EncodingUtf8.hpp"

#include "Engine/Engine.hpp"

#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderDeviceEnums.hpp"
#include "Rendering/UniformBufferData.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/BitmapFont.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/ShaderManager.hpp"

#include "System/File.hpp"

struct MaterialBlock
{
	static const std::size_t BufferSize = 16;

	static UniformBuffer::ScalarUniform<float, 0> shadowOffset;
};

DebugTextRenderer::DebugTextRenderer(
	Allocator* allocator,
	RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	shaderManager(nullptr),
	font(nullptr),
	stringCharCount(0),
	stringData(allocator),
	renderData(allocator),
	scaleFactor(1.0f),
	meshId(MeshId{}),
	materialBufferObjectId(0),
	vertexData(allocator),
	indexData(allocator)
{
}

DebugTextRenderer::~DebugTextRenderer()
{
	allocator->MakeDelete(font);

	if (materialBufferObjectId != 0)
	{
		renderDevice->DestroyBuffers(1, &materialBufferObjectId);
	}
}

void DebugTextRenderer::Initialize(ShaderManager* shaderManager)
{
	this->shaderManager = shaderManager;
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

bool DebugTextRenderer::LoadBitmapFont(TextureManager* textureManager, const char* filePath)
{
	Buffer<char> content(allocator);

	if (File::ReadText(filePath, content))
	{
		font = allocator->MakeNew<BitmapFont>(allocator);
		return font->LoadFromBDF(textureManager, content);
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

		if (meshId.IsValid() == false)
		{
			meshId = meshManager->CreateMesh();
		}

		if (materialBufferObjectId == 0)
		{
			RenderBufferUsage usage = RenderBufferUsage::DynamicDraw;

			renderDevice->CreateBuffers(1, &materialBufferObjectId);

			renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, materialBufferObjectId);
			renderDevice->SetBufferData(RenderBufferTarget::UniformBuffer, MaterialBlock::BufferSize, nullptr, usage);
		}

		CreateAndUploadData();

		const char* shaderPath = "res/shaders/debug/debug_text.shader.json";
		ShaderId shaderId = shaderManager->GetIdByPath(StringRef(shaderPath));

		if (shaderId.IsNull())
			return;

		const ShaderData& shader = shaderManager->GetShaderData(shaderId);

		const TextureUniform* textureUniform = nullptr;
		for (unsigned int i = 0; i < shader.textureUniformCount; ++i)
		{
			UniformDataType type = shader.textureUniforms[i].type;

			if (type == UniformDataType::Tex2D)
			{
				textureUniform = shader.textureUniforms + i;
			}
		}

		renderDevice->DepthTestDisable();

		renderDevice->BlendingEnable();
		renderDevice->BlendFunction(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

		// Use shader
		renderDevice->UseShaderProgram(shader.driverId);

		// Update shadow offset

		unsigned char materialUboBuffer[MaterialBlock::BufferSize];
		Vec2f texSize = font->GetTextureSize();
		MaterialBlock::shadowOffset.Set(materialUboBuffer, 1.0f / texSize.y);

		renderDevice->BindBuffer(RenderBufferTarget::UniformBuffer, materialBufferObjectId);
		renderDevice->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, MaterialBlock::BufferSize, materialUboBuffer);

		renderDevice->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBuffer::MaterialBlock::BindingPoint, materialBufferObjectId);

		// Bind texture
		if (textureUniform != nullptr)
		{
			renderDevice->SetActiveTextureUnit(0);
			renderDevice->BindTexture(RenderTextureTarget::Texture2d, font->GetTextureDriverId());
			renderDevice->SetUniformInt(textureUniform->uniformLocation, 0);
		}

		// Draw

		MeshDrawData* draw = meshManager->GetDrawData(meshId);
		renderDevice->BindVertexArray(draw->vertexArrayObject);
		renderDevice->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);

		// Clear data

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
	data.primitiveMode = RenderPrimitiveMode::Triangles;
	data.vertData = vertexData.GetData();
	data.vertCount = vertexData.GetCount();
	data.idxData = indexData.GetData();
	data.idxCount = indexData.GetCount();

	Engine::GetInstance()->GetMeshManager()->Upload_3f2f(meshId, data, RenderBufferUsage::DynamicDraw);
}
