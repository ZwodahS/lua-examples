-- calculate and apply damage from attacker
-- to target
function applyDamage(attacker, target)
    print("[Lua] in lua function");
    local damage = attacker:getDamage();
    print("[Lua] Damage of attacker is " .. damage);
    target:dealtDamage(damage);
    print("[Lua] Damage dealt");
end

function testcharacter(character)
    local health = character:health();
    print("[Lua] Before setting health value  " .. health);
    local newhealth = character:health(3);
    print("[Lua] After setting health value  " .. newhealth);
end
