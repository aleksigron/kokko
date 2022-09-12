#pragma once

namespace kokko
{

class RenderPass
{
public:
    virtual ~RenderPass() {}

    virtual void SetRenderPipeline() {}

    virtual void EndPass() {}
};

} // namespace kokko
