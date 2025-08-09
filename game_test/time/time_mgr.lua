local os = os;

local time_min = 60;
local time_hour = 60 * time_min;
local time_day = time_hour * 24;
local time_week = time_day * 7; --[[
local start_param = {
    time = {year = 0,month = 0,day = 0,hour = 0,min = 0,sec = 0},
    start_server_after_day = 0, -- 开服几天后
    start_server_after_week = {week = 0, week_day = 0}
};
]]

local mgr = {};

mgr.get_day_start = function(ts)
    local tb = os.date("*t", ts);
    tb.hour, tb.min, tb.sec = 0, 0, 0;
    return os.time(tb);
end

local parse_duration = function(tb)
    local day = tb.day or 0;
    local hour = tb.hour or 0;
    local min = tb.min or 0;
    return day * time_day + hour * time_hour + min * time_min;
end

local start_param_type1 = function(start_ts, cfg, now_ts)
    -- local now_ts = os.time();
    local every = cfg.every;
    local duration = parse_duration(cfg.duration);
    local end_ts = start_ts + duration;

    while true do
        if now_ts < start_ts then
            return start_ts, end_ts;
        elseif now_ts >= start_ts and now_ts < end_ts then
            return start_ts, end_ts;
        end
        if not cfg.next then
            return;
        end
        local next_dura = parse_duration(cfg.next);
        start_ts = end_ts + next_dura;
        end_ts = start_ts + duration;
    end
end

local start_param_everyweek = function(week_arr, dura, now_ts)
    -- local now_ts = os.time();
    local start_end = {};
    local tm_tb = os.date("*t", now_ts);
    tm_tb.hour, tm_tb.min, tm_tb.sec = 0, 0, 0;
    local now_day = tm_tb.day;
    local now_week_day = tm_tb.wday - 1;
    now_week_day = now_week_day == 0 and 7 or now_week_day;
    for _, w in ipairs(week_arr) do
        local day = now_day + w - now_week_day;
        tm_tb.day = day;
        local start_ts = os.time(tm_tb);
        local end_ts = start_ts + dura;
        if now_ts < start_ts then
            return start_ts, end_ts;
        elseif now_ts >= start_ts and now_ts < end_ts then
            return start_ts, end_ts;
        end
        table.insert(start_end, {start_ts, end_ts});
    end
    local fir_ele = start_end[1];
    return fir_ele[1] + time_week, fir_ele[2] + time_week;
end

local handlers = {
    time = function(val, cfg, ts)
        local start_ts = os.time(val);
        return start_param_type1(start_ts, cfg, ts);
    end,
    start_server_after_day = function(val, cfg, ts)
        local start_ts = mgr.start_server_time + val * time_day;
        return start_param_type1(start_ts, cfg, ts);
    end,
    start_server_after_week = function(val, cfg, ts)
        local week, week_day = val.week, val.week_day;
        local start_tm_tb = os.date("*t", mgr.start_server_time);
        local wday = start_tm_tb.wday - 1;
        wday = wday == 0 and 7 or wday;
        if week_day < wday then
            week = week + 1;
        end
        local start_ts = mgr.start_server_time + (week - 1) * time_week + (week_day - wday) * time_day;
        return start_param_type1(start_ts, cfg, ts);
    end,
    every_week = function(val, cfg, ts)
        local dura = parse_duration(cfg.duration);
        return start_param_everyweek(val, dura, ts);
    end
};

-- 如果该活动正在进行，返回当前的开始结束时间，否则，返回下一次的开始结束时间
local get_start_and_end = function(cfg, ts)
    ts = ts or os.time();
    local start_param = cfg.start_param;
    local key, val = next(start_param);
    return handlers[key](val, cfg, ts);
end

mgr.start_server_time = mgr.get_day_start();
mgr.get_start_and_end = get_start_and_end;

return mgr;
