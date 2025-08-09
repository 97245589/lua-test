local cfg = {}

cfg.skill = {
    [1] = {
        action = "add_buff(me, 1, p[1], {p[2], p[3]})",
        params = {{1, "atk_diff", 10}, {3, "atk_diff", 20}}
    },
    [2] = {
        action = "cause_harm(me, targ, p[1])",
        params = {{2}}
    },
    [3] = {
        action = "add_buff(targ, 3, p[1], {p[2]}, me)",
        params = {{3, 0.2}}
    },
    [4] = {
        action = "add_buff(me, 4, 999, {2, p[1]})",
        params = {{2}}
    },
    [41] = {
        action = "add_buff(me, 41, 999, {2, p[1]})",
        params = {{2}}
    },
    [5] = {
        action = [[add_buff(me, 5, 999, {"atk_diff", 10})]],
        params = {{}}
    }
}

cfg.buff = {
    [1] = {
        type1 = "buff",
        action = "buff_attr(p[1], p[2])"
    },
    [2] = {
        type1 = "buff",
        type2 = "forzen",
        type3 = {"not_skill", "not_common_atk"}
    },
    [3] = {
        type1 = "debuff",
        type2 = "poison",
        tick_action = [[
            real_harm(me, get_final_attr(me, "hp")*p[1])
        ]]
    },
    [4] = {
        action = [[
            reg_eve("damage", {exec_skill, nil, nil, p[1], {p[2]}})
        ]]
    },
    [41] = {
        action = [[
            reg_eve("damaged", {exec_skill, nil, nil, p[1], {p[2]}})
        ]]
    },
    [5] = {
        type1 = "buff",
        action = [[
            buff_attr(p[1], p[2]);
            reg_eve("damage", {remove_buff, me, buff.buff_uid})
        ]]
    }
}

return cfg
