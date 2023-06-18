#include "Debug/DebugTextRenderer.hpp"

#include <cassert>
#include <cstdint>

#include "Core/Core.hpp"
#include "Core/EncodingUtf8.hpp"
#include "Core/String.hpp"

#include "Engine/Engine.hpp"

#include "Rendering/CommandEncoder.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderTypes.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/BitmapFont.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/ShaderManager.hpp"

#include "System/Filesystem.hpp"

struct UniformBlock
{
	alignas(16) Vec2f shadowOffset;
};

DebugTextRenderer::DebugTextRenderer(
	Allocator* allocator,
	kokko::render::Device* renderDevice,
	kokko::Filesystem* filesystem) :
	allocator(allocator),
	renderDevice(renderDevice),
	filesystem(filesystem),
	shaderManager(nullptr),
	meshManager(nullptr),
	stringCharCount(0),
	stringData(allocator),
	displayData(allocator),
	scaleFactor(1.0f),
	meshId(kokko::MeshId{}),
	bufferObjectId(0),
	vertexData(allocator),
	indexData(allocator)
{
}

DebugTextRenderer::~DebugTextRenderer()
{
	if (bufferObjectId != 0)
	{
		renderDevice->DestroyBuffers(1, &bufferObjectId);
	}
}

bool DebugTextRenderer::Initialize(kokko::ShaderManager* shaderManager,
	kokko::MeshManager* meshManager, kokko::TextureManager* textureManager)
{
	this->shaderManager = shaderManager;
	this->meshManager = meshManager;

	auto scope = renderDevice->CreateDebugScope(0, kokko::ConstStringView("DebugText_InitResources"));

	const char* const debugFontFilename = "engine/fonts/gohufont-uni-14.bdf";
	if (LoadBitmapFont(textureManager, debugFontFilename) == false)
	{
		KK_LOG_ERROR("Loading debug font failed: {}", debugFontFilename);
		return false;
	}

	return true;
}

void DebugTextRenderer::SetFrameSize(const Vec2f& size)
{
	frameSize = size;
	scaledFrameSize = size * (1.0f / scaleFactor);
}

void DebugTextRenderer::SetScaleFactor(float scale)
{
	scaleFactor = scale;
	scaledFrameSize = frameSize * (1.0f / scaleFactor);
}

size_t DebugTextRenderer::GetRowCountForTextLength(size_t characterCount) const
{
	int glyphWidth = font->GetGlyphWidth();
	int screenWidth = static_cast<int>(scaledFrameSize.x);

	if (screenWidth > 0 && glyphWidth > 0)
	{
		int charsPerRow = screenWidth / glyphWidth;

		return (characterCount + charsPerRow - 1) / charsPerRow;
	}
	else
		return 0;
}

bool DebugTextRenderer::LoadBitmapFont(kokko::TextureManager* textureManager, const char* filePath)
{
	KOKKO_PROFILE_FUNCTION();

	kokko::String content(allocator);

	if (filesystem->ReadText(filePath, content))
	{
		font = kokko::MakeUnique<BitmapFont>(allocator, allocator);
		return font->LoadFromBDF(textureManager, content.GetRef());
	}
	else
		return false;
}

void DebugTextRenderer::AddTextNormalized(kokko::ConstStringView str, Vec2f position)
{
	Vec2f pixelPos = Vec2f::Hadamard(position, scaledFrameSize);
	Rectanglef area(pixelPos, scaledFrameSize);

	this->AddText(str, area);
}

void DebugTextRenderer::AddText(kokko::ConstStringView str, Vec2f position)
{
	Rectanglef area(position, scaledFrameSize);

	this->AddText(str, area);
}

void DebugTextRenderer::AddText(kokko::ConstStringView str, const Rectanglef& area)
{
	stringCharCount += EncodingUtf8::CountCharacters(str);

	size_t stringPosition = stringData.GetCount();
	stringData.InsertBack(str.str, str.len);

	DisplayData& dd = displayData.PushBack();
	dd.stringStart = stringPosition;
	dd.stringLength = str.len;
	dd.area = area;
}

void DebugTextRenderer::Render(kokko::render::CommandEncoder* encoder)
{
	KOKKO_PROFILE_FUNCTION();

	if (displayData.GetCount() > 0 && font != nullptr)
	{
		auto scope = encoder->CreateDebugScope(0, kokko::ConstStringView("DebugText_Render"));

		if (meshId == kokko::MeshId::Null)
		{
			meshId = meshManager->CreateMesh();
		}

		if (bufferObjectId == 0)
		{
			renderDevice->CreateBuffers(1, &bufferObjectId);
			renderDevice->SetBufferStorage(bufferObjectId, sizeof(UniformBlock), nullptr, BufferStorageFlags::Dynamic);
		}

		CreateAndUploadData();

		const char* shaderPath = "engine/shaders/debug/debug_text.glsl";
		kokko::ShaderId shaderId = shaderManager->FindShaderByPath(kokko::ConstStringView(shaderPath));

		if (shaderId == kokko::ShaderId::Null)
			return;

		const kokko::ShaderData& shader = shaderManager->GetShaderData(shaderId);

		kokko::ConstStringView uniformName("glyph_tex");
		const kokko::TextureUniform* textureUniform = shader.uniforms.FindTextureUniformByName(uniformName);

		encoder->DepthTestDisable();

		encoder->BlendingEnable();
		encoder->BlendFunction(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

		// Use shader
		encoder->UseShaderProgram(shader.driverId);

		encoder->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, bufferObjectId);

		// Bind texture
		if (textureUniform != nullptr)
		{
			encoder->BindTextureToShader(textureUniform->uniformLocation, 0, font->GetTextureDriverId());
		}

		// Draw

		const kokko::MeshDrawData* draw = meshManager->GetDrawData(meshId);
		encoder->BindVertexArray(draw->vertexArrayObject);
		encoder->DrawIndexed(draw->primitiveMode, draw->indexType, draw->count, 0, 0);

		// Clear data

		stringCharCount = 0;
		stringData.Clear();
		displayData.Clear();
	}
}

void DebugTextRenderer::CreateAndUploadData()
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = renderDevice->CreateDebugScope(0, kokko::ConstStringView("DebugText_UpdateResources"));

	// Make sure vertex indices fit in unsigned short type
	assert(stringCharCount * 4 < (1 << 16));

	const unsigned int componentCount = 5;
	const unsigned int verticesPerChar = 4;
	const unsigned int floatsPerChar = componentCount * verticesPerChar;
	const unsigned int indicesPerChar = 6;

	vertexData.Resize(stringCharCount * floatsPerChar);
	indexData.Resize(stringCharCount * indicesPerChar);

	int fontLineHeight = font->GetLineHeight();
	Vec2f textureSize = font->GetTextureSize();
	Vec2f invTexSize = Vec2f(1.0f / textureSize.x, 1.0f / textureSize.y);

	Vec2f scaledInvFrame = Vec2f(2.0f / scaledFrameSize.x, 2.0f / scaledFrameSize.y);

	float* vertexBegin = vertexData.GetData();
	float* vertexItr = vertexBegin;
	unsigned short* indexItr = indexData.GetData();

	const char* strData = stringData.GetData();

	DisplayData* ddItr = displayData.GetData();
	DisplayData* ddEnd = ddItr + displayData.GetCount();
	for (; ddItr != ddEnd; ++ddItr)
	{
		DisplayData rd = *ddItr;
		Vec2f drawPos = Vec2f(rd.area.position.x, -rd.area.position.y);

		const char* strItr = strData + rd.stringStart;
		const char* strEnd = strItr + rd.stringLength;
		while (strItr < strEnd)
		{
			uint32_t codepoint;
			size_t bytesDecoded = EncodingUtf8::DecodeCodepoint(strItr, codepoint);

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

					ptrdiff_t floatIndex = vertexItr - vertexBegin;
					unsigned short vertIndex = static_cast<unsigned short>(floatIndex / componentCount);

					for (unsigned int i = 0; i < verticesPerChar; ++i)
					{
						float x = static_cast<float>(i % 2);
						float y = static_cast<float>(i / 2);

						vertexItr[0] = (quadPos.x + x * quadRight) * scaledInvFrame.x - 1.0f;
						vertexItr[1] = (quadPos.y + y * quadDown) * scaledInvFrame.y + 1.0f;
						vertexItr[2] = 0.0f;
						vertexItr[3] = uvX + x * uvRight;
						vertexItr[4] = uvY + y * uvDown;

						vertexItr += componentCount;
					}

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
				else
					vertexItr += floatsPerChar;

				indexItr += 6;

				// TODO: Do we need to have empty char drawing in an else here?
			}
			else // Not a valid UTF8 character, try next byte
			{
				strItr += 1;
			}
		}
	}

	VertexAttribute attributes[] = {
		VertexAttribute::pos3,
		VertexAttribute::uv0
	};
	VertexFormat format(attributes, sizeof(attributes) / sizeof(attributes[0]));
	format.CalcOffsetsAndSizeInterleaved();

	kokko::IndexedVertexData data;
	data.vertexFormat = format;
	data.primitiveMode = RenderPrimitiveMode::Triangles;
	data.vertexData = vertexData.GetData();
	data.vertexDataSize = vertexData.GetCount() * sizeof(vertexData[0]);
	data.vertexCount = vertexData.GetCount() / static_cast<int>(componentCount);
	data.vertexBufferUsage = RenderBufferUsage::DynamicDraw;
	data.indexData = indexData.GetData();
	data.indexDataSize = indexData.GetCount() * sizeof(indexData[0]);
	data.indexCount = static_cast<unsigned int>(indexData.GetCount());
	data.indexBufferUsage = RenderBufferUsage::DynamicDraw;

	meshManager->UploadIndexed(meshId, data);

	// Update shadow offset

	UniformBlock uniforms;
	Vec2f texSize = font->GetTextureSize();
	uniforms.shadowOffset = Vec2f(0.0f, 1.0f / texSize.y);

	renderDevice->SetBufferSubData(bufferObjectId, 0, sizeof(UniformBlock), &uniforms);
}
