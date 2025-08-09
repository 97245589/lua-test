return {
    skill = {
        [1] = {
            targ = { 1, 1 },
            action = "formu_harm(me, targ, p[1])",
            params = { 1 }
        },
        [1000] = {
            action = "add_buff(me, 1000, p[1], {p[2], p[3]})",
            params = { 10, "atk_diff", 10 }
        },
        [1001] = {
            targ = { 1, 1 },
            action = "add_buff(targ, 1000, p[1], {p[2], p[3]})",
            params = { 10, "atk_per", 0.1 }
        },
        [2000] = {
            targ = { 2, 1, 30 },
            action = ""
        }
    },
    buff = {
        [1000] = {
            action = "buff_attr(p[1], p[2])"
        },
        [2000] = {}
    },
    ai = {
        [1] = {
            attrs = { hp = 10, atk = 10, def = 10 },
            behavior = {
                [1] = [[
                    arrive_pos(me, 9, 9)
                ]],
                [2] = [[
                    start_skill(me, 1) and not alive(me)
                ]]
            },
        }
    },
}
