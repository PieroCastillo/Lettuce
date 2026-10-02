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

auto UIScene::getDefStyle() -> std::shared_ptr<Style>
{
    return impl->m_defaultStyle;
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

void UIScene::Build(Controls::Primitives::Control& root)
{
    auto& visualRoot = root;

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
            auto* controlPtr = &control.get();
            instances.push_back({});
            control.get().Build(*impl->m_surface, instances.back());
            // instance functions are copied in Control::Build()
            instances.back().parent = parentIdx;
            queue.push_back(&control.get());
        }
    }
}

void UIScene::Arrange(uint32_t width, uint32_t height)
{
    auto& layoutCtx = impl->m_layoutContext;
    layoutCtx.stack.clear();
    auto& root = impl->m_instances.front();
    layoutCtx.Push(root, { 0.0f, 0.0f, width, height });

    while (!layoutCtx.stack.empty())
    {
        auto& frame = layoutCtx.Current();

        if (frame.instance.layout(layoutCtx))
            layoutCtx.Pop();
    }
}

void UIScene::Update(const InputState& input)
{

}

void UIScene::Record(CommandBuffer& cmd)
{

}