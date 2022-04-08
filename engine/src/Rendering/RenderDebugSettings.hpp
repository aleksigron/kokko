#pragma once

#include "Engine/Entity.hpp"

#include "Rendering/RenderDebugFeature.hpp"

namespace kokko
{

class RenderDebugSettings
{
public:
	RenderDebugSettings();

	Entity GetDebugEntity() const;
	void SetDebugEntity(Entity entity);

	bool IsFeatureEnabled(RenderDebugFeatureFlag feature) const;
	void SetFeatureEnabled(RenderDebugFeatureFlag feature, bool enabled);

private:
	Entity debugEntity;
	uint32_t featureFlags;
};

}
