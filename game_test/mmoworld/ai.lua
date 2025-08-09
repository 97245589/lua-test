local ai_cfg = require"config".ai
local ai_env = {
    world = nil,
    me = nil,
    skill_env = nil,
    diff_tm = nil
}

local init_ai = function()
    for aiid, cfg in pairs(ai_cfg) do
        local beh = cfg.behavior
        for idx, str in pairs(beh) do
            str = "return " .. str
            beh[idx] = load(str, string.format("aicfg %d %d", aiid, idx), "t", ai_env)
        end
    end
end
init_ai()

ai_env.is_near = function(px, py, ex, ey)
    if math.abs(px - ex) < 1e-6 and math.abs(py - ey) < 1e-6 then
        return true
    end
    return false
end

local arrive_pos = function(me, ex, ey, dis)
end

ai_env.arrive_pos = function(me, ex, ey, diff_tm)
    local world = ai_env.world
    if ai_env.is_near(me.px, me.py, ex, ey) then
        me.path = nil
        me.now_path_idx = nil
        return true
    end

    if not me.path then
        me.path = world.path_find(me.px, me.py, ex, ey)
        me.now_path_idx = 0
        return false
    end

    me.now_path_idx = me.now_path_idx + 2
    local path, idx = me.path, me.now_path_idx
    local nx, ny, nnx, nny = path[idx + 1], path[idx + 2], path[idx + 3], path[idx + 4]

    me.px, me.py = nx, ny
    if ai_env.is_near(me.px, me.py, ex, ey) then
        me.path = nil
        me.now_path_idx = nil
        return true
    end

    me.dx, me.dy = nnx - nx, nny - ny
    world.update_pos(me.id, me.mode, me.px, me.py, me.dx, me.dy, me.speed)
end

ai_env.start_skill = function(me, skillid)
    local skill_env = ai_env.skill_env
    skill_env.start_skill(me, skillid)
end

ai_env.alive = function(me)
    return me.attrs.hp > 0
end

ai_env.exec_ai = function()
    local me = ai_env.me
    local aiid = me.aiid
    local beh = ai_cfg[aiid].behavior
    for idx, func in ipairs(beh) do
        if not func() then
            -- print("aifunc", idx)
            return
        end
    end
end

return {
    ai_env = ai_env
}
