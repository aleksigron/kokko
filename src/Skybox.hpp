#pragma once

struct Texture;

struct Skybox
{
	unsigned int textureDriverId;
	unsigned int meshDriverId;

	Skybox();

	void SetTexture(const Texture* texture);
};
