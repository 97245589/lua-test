return {
    skill = {
        [1] = {
            action = "harm_formular(me, targ, p)"
        },
        [1001] = {
            type = "active_harm",
            targets = {
                me = false,
                forward = true,
                num = 1
            },
            action = [[harm_formular(me, targ, {
                damage_c = p[1] or 1,
                fdamage = 0,
            })]],
            round_start_action = nil,
            round_end_action = nil,
            tick_start_action = nil,
            tick_end_action = nil
        },
        [2001] = {
            type = "passive"
            -- action = "add_buff(me, 1, 60, {p[1], p[2]})",
        },
        [101001] = {
            type = "passive"
            -- type = "active_buff",
            -- targets = {me = true, num = 3},
            -- action = [[
            --     add_hp(targ, 10)
            --     add_buff(targ, 1, 3, {"atk", 10}, me)
            -- ]]
        },
        [102001] = {}
    },

    buff = {
        [1] = {
            type1 = "buff",
            action = "buff_attr(p[1], p[2])"
        },
        [2] = {
            type1 = "debuff",
            type2 = "stun"
        }
    }
}
