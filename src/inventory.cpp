#include "Inventory.h"
#include "raylib.h" // Only needed for DrawText

void Inventory::AddItem(const std::string& itemId) {
    items[itemId]++;
}

bool Inventory::UseItem(const std::string& itemId) {
    auto it = items.find(itemId);
    if (it != items.end() && it->second > 0) {
        it->second--;
        return true;
    }
    return false;
}

bool Inventory::HasItem(const std::string& itemId) const {
    auto it = items.find(itemId);
    return (it != items.end() && it->second > 0);
}

int Inventory::GetItemCount(const std::string& itemId) const {
    auto it = items.find(itemId);
    return (it != items.end()) ? it->second : 0;
}

void Inventory::DrawInventoryUI(int x, int y) const {
    int spacing = 24;
    for (const auto& [id, count] : items) {
        //DrawText(TextFormat("%s x%d", id.c_str(), count), x, y, 20, WHITE);
        y += spacing;
    }
}

void Inventory::DrawInventoryUIWithIcons(Texture2D healthPotionTex, int x, int y, int slotSize) const {
    int spacing = slotSize + 10;

    for (int i = 0; i < 4; ++i) {
        // Slot position
        Rectangle slotRect = { (float)(x + i * spacing), (float)y, (float)slotSize, (float)slotSize };
        
        // Draw empty slot outline
        DrawRectangleLinesEx(slotRect, 2, WHITE);

        // Draw potion if we have it and this is slot 0
        if (i == 0 && HasItem("HealthPotion")) {
            DrawTexturePro(
                healthPotionTex,
                { 0, 0, (float)healthPotionTex.width, (float)healthPotionTex.height },
                { slotRect.x, slotRect.y, slotRect.width, slotRect.height },
                { 0, 0 },
                0.0f,
                WHITE
            );

            // Draw quantity number
            int count = GetItemCount("HealthPotion");
            DrawText(TextFormat("x%d", count), (int)slotRect.x + 4, (int)slotRect.y + slotSize - 20, 16, WHITE);
        }

        // TODO: add more items to other slots later
    }
}
