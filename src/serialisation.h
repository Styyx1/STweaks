#pragma once
#include "Events.h"

// Credits: https://github.com/colinswrath/BladeAndBlunt/blob/main/include/Serialization.h
namespace Serialisation
{
    static constexpr std::uint32_t SerializationVersion = 1;
    static constexpr std::uint32_t ID = 'STWK';
    static constexpr std::uint32_t SerializationType = 'STWD';

    inline void SaveCallback(SKSE::SerializationInterface *a_skse)
    {

        if (!a_skse->OpenRecord(SerializationType, SerializationVersion))
        {
            logger::error("Failed to open STweaks record");
            return;
        }
        else
        {
            auto diseaseManager = Events::HitEventHandler::GetSingleton();
            auto healthdebuffToSerialise = diseaseManager->storedHealth_disease;
            auto staminadebuffToSerialise = diseaseManager->storedStamina_disease;
            auto magickadebuffToSerialise = diseaseManager->storedMagicka_disease;

            if (!a_skse->WriteRecordData(healthdebuffToSerialise))
            {
                logger::error("Failed to write size of record data");
                return;
            }

            if (!a_skse->WriteRecordData(staminadebuffToSerialise))
            {
                logger::error("Failed to write size of record data");
                return;
            }

            if (!a_skse->WriteRecordData(magickadebuffToSerialise))
            {
                logger::error("Failed to write size of record data");
                return;
            }
            else
            {
                logger::info("serialised Health Penalty: {}", healthdebuffToSerialise);
                logger::info("serialised Stamina Penalty: {}", staminadebuffToSerialise);
                logger::info("serialised Magicka Penalty: {}", magickadebuffToSerialise);
            }
        }
    }

    inline void LoadCallback(SKSE::SerializationInterface *a_skse)
    {
        std::uint32_t type;
        std::uint32_t version;
        std::uint32_t length;
        a_skse->GetNextRecordInfo(type, version, length);

        auto diseaseManager = Events::HitEventHandler::GetSingleton();

        if (type != SerializationType)
        {
            return;
        }

        if (version != SerializationVersion)
        {
            logger::error("Unable to load data");
            return;
        }

        float deserialisedHealth;
        float deserialisedStamina;
        float deserialisedMagicka;

        if (!a_skse->ReadRecordData(deserialisedHealth))
        {
            logger::error("Failed to load size");
            return;
        }

        if (!a_skse->ReadRecordData(deserialisedStamina))
        {
            logger::error("Failed to load size");
            return;
        }

        if (!a_skse->ReadRecordData(deserialisedMagicka))
        {
            logger::error("Failed to load size");
            return;
        }
        else
        {
            diseaseManager->storedHealth_disease = deserialisedHealth;
            diseaseManager->storedStamina_disease = deserialisedStamina;
            diseaseManager->storedMagicka_disease = deserialisedMagicka;

            logger::info("Deserialised Health Penalty: {}", deserialisedHealth);
            logger::info("Deserialised Stamina Penalty: {}", deserialisedStamina);
            logger::info("Deserialised Magicka Penalty: {}", deserialisedMagicka);
        }
    }

    inline void RevertCallback([[maybe_unused]] SKSE::SerializationInterface *a_skse)
    {
        auto diseaseManager = Events::HitEventHandler::GetSingleton();
        diseaseManager->storedHealth_disease = 0.0f;
        diseaseManager->storedStamina_disease = 0.0f;
        diseaseManager->storedMagicka_disease = 0.0f;
    }
}