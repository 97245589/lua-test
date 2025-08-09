local time_config = require "time_config"
local time_mgr = require "time_mgr"
require "print"

local print_time = function(ts)
    print(os.date("%Y-%m-%d %H:%M:%S", ts));
end

local ts = os.time({
    year = 2023,
    month = 2,
    day = 8
});

for k, v in pairs(time_config) do
    print("------------ id", k);
    local start_ts, end_ts = time_mgr.get_start_and_end(v, ts);
    if not start_ts then
        goto END
    end
    -- print(start_ts, end_ts);
    print_time(start_ts);
    print_time(end_ts);
    ::END::
end
