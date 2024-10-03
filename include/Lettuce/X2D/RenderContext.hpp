//
// Created by piero on 30/09/2024.
//

namespace Lettuce::X2D
{
    class RenderContext
    {
    public:
        RenderContext();
        void Record(VkCommandBuffer cmd);
    }
}