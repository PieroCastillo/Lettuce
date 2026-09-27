#include <queue>

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
    auto controls = std::vector<ControlInstance>();

    // std::queue<std::unique_ptr<Control>> pendingControls;
    // pendingControls.push(std::move(uiViewDesc.child));

    // while (!pendingControls.empty())
    // {
    //     if (auto* control = dynamic_cast<ContentControl*>(pendingControls.front().get()))
    //     {
    //         pendingControls.push(std::move(control->GetControl(control->Content)));
    //         pendingControls.pop();
    //     }
    //     else if (auto* control = dynamic_cast<ItemControl*>(pendingControls.front().get()))
    //     {
    //         for(auto& item : control->Items.GetData())
    //         {
    //             pendingControls.emplace(control->ItemTemplate(item));
    //         }
    //         pendingControls.pop();
    //     }
    //     else if (auto* control = dynamic_cast<ViewControl*>(pendingControls.front().get()))
    //     {
    //         for(auto& child : control->Children)
    //         {
    //             pendingControls.push(std::move(child));
    //         }
    //         pendingControls.pop();
    //     }
    //     else
    //     {

    //     }
    // }
}

void UIScene::Update()
{

}

void UIScene::Record(CommandBuffer& cmd)
{

}