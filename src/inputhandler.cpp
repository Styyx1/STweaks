#include "inputhandler.h"
namespace Input
{
    void InputHandler::Register()
    {
        if (const auto scripts = RE::BSInputDeviceManager::GetSingleton())
        {
            scripts->AddEventSink<RE::InputEvent *>(GetSingleton());
            logger::info("Registered for {}", typeid(RE::InputEvent).name());
        }
    }

    void InputHandler::RequiemDodge(const hotkeys::KeyCombination *key)
    {
        RE::PlayerCharacter *p = Cache::GetPlayerSingleton();
        if (!Utility::ActorHasActiveEffect(p, Settings::Forms::dodge_perk_effect))
        {
            Utility::ApplySpell(p, p, Settings::Forms::dodge_cost_spell);
            logger::debug("applied {} to {}", Settings::Forms::dodge_cost_spell->GetName(), p->GetDisplayFullName());
        }
        return;
    }

    void InputHandler::SetSprintKey()
    {
        const RE::ControlMap *cm = RE::ControlMap::GetSingleton();
        const RE::UserEvents *userEvent = RE::UserEvents::GetSingleton();
        const auto &sprint_def = RE::UserEvents::GetSingleton()->run;
        auto button = cm->GetUserEventName(42, RE::INPUT_DEVICE::kKeyboard);
        logger::info("myserious event i search for is: {}", button);

        for (int i = RE::INPUT_DEVICE::kKeyboard; i <= RE::INPUT_DEVICE::kGamepad; ++i)
        {
            switch (i)
            {
            case RE::INPUT_DEVICE::kKeyboard:
                if (cm->GetMappedKey(sprint_def, RE::INPUT_DEVICE::kKeyboard) != 0)
                {
                    logger::debug("no keyboard key mapped");
                    break;
                }
                else
                {
                    sprint_key.SetPattern(hotkeys::details::GetNameByKey(cm->GetMappedKey(sprint_def, RE::INPUT_DEVICE::kKeyboard)));
                    logger::debug("KeyCode for keyboard block is {} \n", sprint_key.GetPattern());
                    break;
                }
            case RE::INPUT_DEVICE::kMouse:
                sprint_key_mouse.SetPattern(hotkeys::details::GetNameByKey(SKSE::InputMap::kMacro_MouseButtonOffset + cm->GetMappedKey(sprint_def, RE::INPUT_DEVICE::kMouse)));
                logger::debug("KeyCode for mouse block is {} \n ", sprint_key_mouse.GetPattern());
                break;
            case RE::INPUT_DEVICE::kGamepad:
                sprint_key_controller.SetPattern(hotkeys::details::GetNameByKey(SKSE::InputMap::GamepadMaskToKeycode(cm->GetMappedKey(sprint_def, RE::INPUT_DEVICE::kGamepad))));
                logger::debug("KeyCode for Gamepad block is {} \n", sprint_key_controller.GetPattern());
                break;
            }
        }
    }

    RE::BSEventNotifyControl InputHandler::ProcessEvent(RE::InputEvent *const *a_event, RE::BSTEventSource<RE::InputEvent *> *a_eventSource)
    {
        using EventType = RE::INPUT_EVENT_TYPE;
        using Result = RE::BSEventNotifyControl;

        if (!a_event)
            return Result::kContinue;

        const auto ui = RE::UI::GetSingleton();
        if (ui->GameIsPaused())
        {
            return Result::kContinue;
        }
        sprint_key.Process(a_event);
        sprint_key_mouse.Process(a_event);
        sprint_key_controller.Process(a_event);

        return Result::kContinue;
    }
    void MenuManager::RegisterMenuEvents()
    {
        if (const auto scripts = RE::UI::GetSingleton())
        {
            scripts->AddEventSink<RE::MenuOpenCloseEvent>(GetSingleton());
            logger::info("Registered {}"sv, typeid(RE::MenuOpenCloseEvent).name());
        }
    }
    void MenuManager::SetSprintKey()
    {
        logger::debug("Started Looking up the block key...");
        InputHandler *im = InputHandler::GetSingleton();
        im->SetSprintKey();
    }
    RE::BSEventNotifyControl MenuManager::ProcessEvent(const RE::MenuOpenCloseEvent *event, RE::BSTEventSource<RE::MenuOpenCloseEvent> *a_eventSource)
    {
        using Result = RE::BSEventNotifyControl;
        if (!event)
        {
            return Result::kContinue;
        }
        if (event->menuName != RE::JournalMenu::MENU_NAME)
        {
            return Result::kContinue;
        }
        if (!event->opening)
        {
            SetSprintKey();
        }
        return Result::kContinue;
    }
    void RegisterEvents()
    {
        if (Settings::Values::use_requiem_stamina.GetValue())
        {
            Input::InputHandler::Register();
            Input::MenuManager::RegisterMenuEvents();
        }
        return;
    }
}
