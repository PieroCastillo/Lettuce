#include "Lettuce/UI/api.hpp"
#include "Lettuce/UI/UISceneImpl.hpp"

using namespace Lettuce::UI;
using namespace Lettuce::Quimera;

UIScene::UIScene(std::move_only_function<UIView(void)> builder)
{
    impl = new UISceneImpl;
    Build(std::move(builder));
}

UIScene::~UIScene()
{
    delete impl;
}

void UIScene::Build(std::move_only_function<UIView(void)> builder)
{
    auto uiViewDesc = builder();
}

void UIScene::Update()
{

}

void UIScene::Record(CommandBuffer& cmd)
{

}