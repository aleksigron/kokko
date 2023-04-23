#include "Rendering/RenderResourceId.hpp"

#define DEFINE_DEFAULT_NAME(type, defaultName) const type type::defaultName = type{0}

namespace kokko
{
namespace render
{

DEFINE_DEFAULT_NAME(BufferId, Null);
DEFINE_DEFAULT_NAME(FramebufferId, Default);
DEFINE_DEFAULT_NAME(SamplerId, Null);
DEFINE_DEFAULT_NAME(ShaderId, Null);
DEFINE_DEFAULT_NAME(TextureId, Null);
DEFINE_DEFAULT_NAME(VertexArrayId, Null);

} // namespace render
} // namespace kokko
