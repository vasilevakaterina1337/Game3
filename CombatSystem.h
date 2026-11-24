#pragma once
#include "Unit.h"

// Не содержит своей формулы: делегирует Unit -> Attack -> Armor, где броня вычитается один раз.
class CombatSystem {
public:
    int calculateDamage(const Unit& attacker, const Unit& target,
        int attackBonus = 0, int armorBonus = 0) const {
        return attacker.damageAgainst(target, attackBonus, armorBonus);
    }
};
