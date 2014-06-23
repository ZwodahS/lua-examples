-- calculate and apply damage from attacker
-- to target
function applyDamage(attacker, target)
    print("[Lua] in lua function");
    local damage = attacker.getDamage(attacker);
    print("[Lua] Damage of attacker is " .. damage);
    target:dealtDamage(damage);
    print("[Lua] Damage dealt");
end
