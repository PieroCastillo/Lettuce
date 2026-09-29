// project headers
#include "Lettuce/UI/api.hpp"
#include "Lettuce/UI/UISceneImpl.hpp"

using namespace Lettuce::Quimera;
using namespace Lettuce::UI;
using namespace Lettuce::UI::Controls;
using namespace Lettuce::UI::Controls::Primitives;

auto UIScene::alloc(size_t Tsize, size_t Talignment) -> void*
{
    return impl->m_allocator.allocate(Tsize, Talignment);
}

UIScene::UIScene(const UISceneDesc& desc)
{
    if (impl)
        throw std::logic_error("UIScene::UIScene cannot be called from initialized UIScene.");

    auto nimpl = new UISceneImpl;

    try
    {
        nimpl->Create(desc);
    }
    catch (...)
    {
        delete nimpl;
        throw;
    }
    impl = nimpl;
}

UIScene::~UIScene()
{
    if (impl)
    {
        impl->Destroy();
        delete impl;
    }
    impl = nullptr;
}

UIScene::UIScene(UIScene&& other) noexcept : impl(std::exchange(other.impl, nullptr))
{
}

UIScene& UIScene::operator=(UIScene&& other) noexcept
{
    if (this != &other)
    {
        if (impl)
        {
            impl->Destroy();
            delete impl;
        }
        impl = std::exchange(other.impl, nullptr);
    }
    return *this;
}

void UIScene::Build(std::weak_ptr<Controls::Primitives::Control> root)
{
    auto& visualRoot = *root.lock().get();

    // usually the Control Count per Build() is similar,
    // so reuse vectors is convenient
    auto& instances = impl->m_instances;
    auto& queue = impl->m_tempQueue;
    instances.clear();
    queue.clear();

    instances.push_back({});
    visualRoot.Build(*impl->m_surface, instances.back());
    instances.back().parent = InvalidControlInstance;
    queue.push_back(&visualRoot);

    for (uint32_t parentIdx = 0; parentIdx < queue.size(); ++parentIdx)
    {
        auto& parent = *queue[parentIdx];
        const auto children = parent.Children();

        if (children.empty())
            continue;

        instances[parentIdx].firstChild = (uint32_t)instances.size();
        instances[parentIdx].childrenCount = (uint32_t)children.size();

        for (auto& control : children)
        {
            instances.push_back({});
            control.get().Build(*impl->m_surface, instances.back());
            instances.back().parent = parentIdx;
            queue.push_back(&control.get());
        }
    }
}

void UIScene::Update(const InputState& input)
{

}

void UIScene::Record(CommandBuffer& cmd)
{

}