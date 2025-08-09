local time_min = 60
local time_hour = 60 * 60
local time_day = 24 * 60 * 60
local time_week = 7 * 24 * 60 * 60

local os, print = os, print;

local day_start = function(t)
    local tb = os.date("*t", t);
    tb.hour = 0;
    tb.min = 0;
    tb.sec = 0;
    return os.time(tb);
end

-- 默认为周一0点
local week_start = function(t)
    local tb = os.date("*t", t);
    local week = tb.wday - 1;
    week = week == 0 and 7 or week;
    tb.hour = 0;
    tb.min = 0;
    tb.sec = 0;
    tb.day = tb.day - (week - 1);
    return os.time(tb);
end

local is_sameday = function(t1, t2)
    return day_start(t1) == day_start(t2);
end

local is_sameweek = function(t1, t2)
    return week_start(t1) == week_start(t2);
end

local print_date = function(t)
    print(os.date("%Y-%m-%d %H:%M:%S", t));
end
