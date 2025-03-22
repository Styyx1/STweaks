#pragma once
#include "Settings.h"
#include <Xinput.h>
#include <CLIBUtil/hotkeys.hpp>
#include "cache.h"
#include "Utility.h"

namespace Input
{
    void RegisterEvents();

    class InputHandler : public REX::Singleton<InputHandler>,
                         public RE::BSTEventSink<RE::InputEvent *>
    {
    public:
        static void Register();
        static void RequiemDodge(const hotkeys::KeyCombination *key);
        hotkeys::KeyCombination sprint_key{RequiemDodge};
        hotkeys::KeyCombination sprint_key_mouse{RequiemDodge};
        hotkeys::KeyCombination sprint_key_controller{RequiemDodge};
        void SetSprintKey();

    protected:
        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent *const *a_event, RE::BSTEventSource<RE::InputEvent *> *a_eventSource) override;

    private:
    };
    class MenuManager : public REX::Singleton<MenuManager>,
                        public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {

    public:
        static void RegisterMenuEvents();
        static void SetSprintKey();

    protected:
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent *event, RE::BSTEventSource<RE::MenuOpenCloseEvent> *a_eventSource) override;
    };
}