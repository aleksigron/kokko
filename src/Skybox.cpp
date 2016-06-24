#include "Skybox.hpp"

#include "Texture.hpp"

Skybox::Skybox() : textureDriverId(0), meshDriverId(0)
{
}

void Skybox::SetTexture(const Texture* texture)
{
	textureDriverId = texture->driverId;
}
