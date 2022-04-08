#include "Rendering/RenderDebugSettings.hpp"

namespace kokko
{

RenderDebugSettings::RenderDebugSettings() :
	debugEntity(Entity::Null),
	featureFlags(0)
{
}

Entity RenderDebugSettings::GetDebugEntity() const
{
	return debugEntity;
}

void RenderDebugSettings::SetDebugEntity(Entity entity)
{
	debugEntity = entity;
}

bool RenderDebugSettings::IsFeatureEnabled(RenderDebugFeatureFlag feature) const
{
	uint32_t feat = static_cast<uint32_t>(feature);
	return (feat & featureFlags) == feat;
}

void RenderDebugSettings::SetFeatureEnabled(RenderDebugFeatureFlag feature, bool enabled)
{
	if (enabled)
		featureFlags |= static_cast<uint32_t>(feature);
	else
		featureFlags &= ~static_cast<uint32_t>(feature);
}

}
