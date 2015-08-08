#include "App.h"

#include "GeometryBuilder.h"
#include "ImageData.h"
#include "JsonReader.h"
#include <string>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

App* App::instance = nullptr;

App::App()
{
	App::instance = this;
}

App::~App()
{
}

static void ReadString(const JsonReader::String& key,
					   const JsonReader::String& value,
					   void* userData)
{
	std::string k;
	k.append(key.str, key.len);

	std::string v;
	v.append(value.str, value.len);

	int i = 0;
}

static void OpenObject(void* userData)
{
	int i(0);
}

static void CloseObject(void* userData)
{
	int i(0);
}

bool App::Initialize()
{
	if (this->mainWindow.Initialize())
	{
		this->renderer.Initialize();
		this->renderer.AttachTarget(&this->mainWindow);
		this->renderer.SetActiveCamera(&this->mainCamera);

		const char* json = "{\"vertexShaderFile\": \"res/shaders/simple.vert\", \"fragmentShaderFile\": \"res/shaders/simple.frag\"}";
		JsonReader reader(json);
		JsonReader::CallbackInfo cb;
		cb.readString = ReadString;
		cb.openObject = OpenObject;
		cb.closeObject = CloseObject;
		reader.Parse(cb);

		// Test image
		ImageData image;
		image.LoadPng("res/textures/test.png");

		ObjectId texId = this->resourceManager.textures.Add();
		Texture& tex = this->resourceManager.textures.Get(texId);
		tex.Upload(image);

		image.DeallocateData();

		// Color shader
		ObjectId colShaderId = resourceManager.shaders.Add();
		ShaderProgram& colShader = resourceManager.shaders.Get(colShaderId);
		colShader.Load("res/shaders/simple.vert", "res/shaders/simple.frag");

		// Color material
		ObjectId colorMaterialId = resourceManager.materials.Add();
		Material& colorMaterial = resourceManager.materials.Get(colorMaterialId);
		colorMaterial.shader = colShaderId;
		colorMaterial.uniformCount = 0;

		// Color object
		this->cube0 = GeometryBuilder::UnitCubeWithColor();
		RenderObject& cube0obj = this->renderer.GetRenderObject(this->cube0);
		cube0obj.material = colorMaterialId;

		// Texture shader
		ObjectId texShaderId = resourceManager.shaders.Add();
		ShaderProgram& texShader = resourceManager.shaders.Get(texShaderId);
		texShader.Load("res/shaders/tex.vert", "res/shaders/tex.frag");

		// Texture material
		ObjectId texMaterialId = resourceManager.materials.Add();
		Material& texMaterial = resourceManager.materials.Get(texMaterialId);
		texMaterial.shader = texShaderId;
		texMaterial.uniformCount = 1;
		texMaterial.uniforms[0].location = glGetUniformLocation(texShader.oglId, "tex");
		texMaterial.uniforms[0].type = ShaderUniformType::Texture2D;
		texMaterial.uniforms[0].dataOffset = 0;

		texMaterial.uniformData = new unsigned char[4];

		unsigned* textureIdPtr = reinterpret_cast<unsigned*>(texMaterial.uniformData);
		*textureIdPtr = tex.textureGlId;

		// Second cube

		this->cube1 = GeometryBuilder::UnitCubeWithTextureCoords();
		RenderObject& cube1obj = this->renderer.GetRenderObject(this->cube1);
		cube1obj.material = texMaterialId;

		this->mainCamera.transform.position = Vec3f(0.0f, 0.0f, 5.0f);

		Vec2i frameSize = this->mainWindow.GetFrameBufferSize();
		this->mainCamera.SetAspectRatio(frameSize.x, frameSize.y);
		
		return true;
	}
	else
		return false;
}

bool App::HasRequestedQuit()
{
	return this->mainWindow.ShouldClose();
}

void App::Update()
{
	this->time.Update();

	float t = Time::GetTime();
	
	RenderObject& cube0obj = this->renderer.GetRenderObject(this->cube0);
	cube0obj.transform.position = Vec3f(cosf(t * 0.5f) * 1.2f, sinf(t * 0.5f) * 1.2f, 0.0f);
	cube0obj.transform.rotation = Matrix::Rotate(Vec3f(1.0f, 1.0f, 1.0f), t * -1.0f);

	RenderObject& cube1obj = this->renderer.GetRenderObject(this->cube1);
	cube1obj.transform.rotation = Matrix::Rotate(Vec3f(1.0f, 1.0f, -1.0f), t * 1.0f);
	cube1obj.transform.position = Vec3f(cosf(t * 0.5f) * -1.2f, sinf(t * 0.5f) * -1.2f, 0.0f);

	this->renderer.Render();
	this->mainWindow.Swap();
}
