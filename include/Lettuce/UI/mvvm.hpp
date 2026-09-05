/*
Created by @PieroCastillo on 2026-09-05
*/
#ifndef LETTUCE_UI_MVVM_HPP
#define LETTUCE_UI_MVVM_HPP

#include <functional>
#include <vector>

namespace Lettuce::UI
{
    template<typename T>
    using subscriber = std::move_only_function<void(const T&)>;

    template<typename T>
    class Observable
    {
    private:
        T data;
        std::vector<subscriber<T>> subscribers;

        void notify()
        {
            for (auto& sub : subscribers)
                sub(data);
        }
    public:
        auto Get() -> T
        {
            return data;
        }
        void Set(T value)
        {
            data = value;
            notify();
        }
    };

    template<typename T>
    class ObservableVector
    {
    private:
        std::vector<T> data;
        std::vector<subscriber<std::vector<T>>> subscribers;

        void notify() {
            for (auto& sub : subscribers)
                sub(data);
        }
    public:
        void PushBack(const T& value) {
            data.push_back(value);
            notify();
        }

        void PopBack() {
            if (!data.empty()) {
                data.pop_back();
                notify();
            }
        }

        const std::vector<T>& GetData() const {
            return data;
        }
    };

    template<typename T>
    class RelayCommand
    {
    private:
        std::move_only_function<T()> executeAction;
        std::move_only_function<bool()> canExecuteAction;
    public:
        RelayCommand(std::move_only_function<T()>&& execute, std::move_only_function<bool()>&& canExecute) :
            executeAction(std::move(execute)), canExecuteAction(std::move(canExecute))
        {
        }

        void Execute()
        {
            if(canExecuteAction())
                executeAction();
        }

        auto CanExecute() -> bool 
        {
            return canExecuteAction ? canExecuteAction() : true;
        }
    };
};
#endif // LETTUCE_UI_MVVM_HPP