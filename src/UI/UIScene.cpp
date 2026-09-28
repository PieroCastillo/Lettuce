// project headers
#include "Lettuce/UI/api.hpp"
#include "Lettuce/UI/UISceneImpl.hpp"

using namespace Lettuce::UI;
using namespace Lettuce::Quimera;

auto UIScene::alloc(size_t Tsize, size_t Talignment) -> void*
{
    return impl->m_allocator.allocate(Tsize, Talignment);
}

UIScene::UIScene()
{
    if (impl)
        throw std::logic_error("UIScene::UIScene cannot be called from initialized UIScene.");

    auto nimpl = new UISceneImpl;

    try
    {
        impl->Create();
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

void UIScene::Build(std::weak_ptr<Controls::Primitives::Control> visualRoot)
{

}

void UIScene::Update(const InputState& input)
{

}

void UIScene::Record(CommandBuffer& cmd)
{

}