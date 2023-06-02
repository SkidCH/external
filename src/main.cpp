#include <thread>
#include <iostream>

#include "memory.h"
#include "../ext/offsets.h"

int main() {
    auto memory = Memory{ "csgo.exe" };
    const auto client = memory.GetModuleAddress("client.dll");
    const auto engine = memory.GetModuleAddress("engine.dll");

    while (true)
    {
        const auto localPlayer = memory.Read<std::uintptr_t>(client + offsets::dwLocalPlayer);
        const auto glowObjectManager = memory.Read<std::uintptr_t>(client + offsets::dwGlowObjectManager);
        const auto& localHealth = memory.Read<std::int32_t>(localPlayer + offsets::m_iHealth);

        for (auto i = 0; i < 64; ++i)
        {
            const auto entity = memory.Read<std::uintptr_t>(client + offsets::dwEntityList + i * 0x10);
            const auto teamNum = memory.Read<std::uintptr_t>(localPlayer + offsets::m_iTeamNum);
            if (memory.Read<std::uintptr_t>(entity + offsets::m_iTeamNum) == teamNum)
                continue;

            const auto glowIndex = memory.Read<std::int32_t>(entity + offsets::m_iGlowIndex);
            const auto glowObjectManagerOffset = glowObjectManager + (glowIndex * 0x38);

            memory.Write<float>(glowObjectManagerOffset + 0x8, 0.f); // red
            memory.Write<float>(glowObjectManagerOffset + 0xC, 0.f); // green
            memory.Write<float>(glowObjectManagerOffset + 0x10, 1.f); // blue
            memory.Write<float>(glowObjectManagerOffset + 0x14, 0.6f); // Brightness

            memory.Write<bool>(glowObjectManagerOffset + 0x27, true);
            memory.Write<bool>(glowObjectManagerOffset + 0x28, true);
        }

        if (!GetAsyncKeyState(VK_XBUTTON2)) {
            continue;
        }
        else if (!localHealth) {
            continue;
        }
        else {
            const auto& crosshairId = memory.Read<std::int32_t>(localPlayer + offsets::m_iCrosshairId);

            if (!crosshairId || crosshairId > 64) {
                continue;
            }
            else {
                const auto& player = memory.Read<std::uintptr_t>(client + offsets::dwEntityList + (crosshairId - 1) * 0x10);

                if (!memory.Read<std::int32_t>(player + offsets::m_iHealth)) {
                    continue;
                }
                else {
                    const auto& playerTeamNum = memory.Read<std::int32_t>(player + offsets::m_iTeamNum);
                    const auto& localPlayerTeamNum = memory.Read<std::int32_t>(localPlayer + offsets::m_iTeamNum);
                    if (playerTeamNum == localPlayerTeamNum) {
                        continue;
                    }
                    else {
                        memory.Write<std::uintptr_t>(client + offsets::dwForceAttack, 6);
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        memory.Write<std::uintptr_t>(client + offsets::dwForceAttack, 4);
                    }
                }
            }
        }
    }
    return 0;
}