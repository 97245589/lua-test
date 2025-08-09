local require, type, load = require, type, load
local table, pairs, next = table, pairs, next
require "util"
local print, print_v, dump = print, print_v, dump

local enums = {
    task1 = 1,

    taskstatus_unfinish = 1,
    taskstatus_finish = 2,

    tasktarg_build = 1,

    build_main = 1,
    build_sec = 2
}

local task1_cfg = {
    [10] = { -- event = {"tasktarg_build", "build_main", 5}
        event = {1, 1, 5}
    },
    [11] = {
        event = {1, 1, 10}
    },
    [20] = { -- event = {"tasktarg_build", "build_sec", 10}
        event = {1, 2, 10}
    }
}

local create_task_mgrs = function(init_handle)
    local gen_event_mark = function(event_arr)
        return table.concat(event_arr, "|", 1, #event_arr - 1)
    end

    local task_check_num = function(task_obj, task, cfg_event, del_marks)
        if task.status ~= enums.taskstatus_unfinish then
            return
        end
        local num = cfg_event[#cfg_event]
        if task.num >= num then
            task.num = num
            local taskid = task.id
            task.status = enums.taskstatus_finish

            if not del_marks then
                return
            end
            local marks = task_obj.marks
            local task_mark = gen_event_mark(cfg_event)
            marks[task_mark][taskid] = nil
            if not next(marks[task_mark]) then
                marks[task_mark] = nil
            end
        end
    end

    local task_mgrs = {}
    local ret = {}

    local init_one_task = function(obj, task_obj, taskid, tcfg)
        local tasks = task_obj.tasks
        if tasks[taskid] then
            return
        end
        local task = {
            id = taskid,
            num = 0,
            status = enums.taskstatus_unfinish
        }

        local cevent = tcfg.event
        local handle = init_handle
        local ep1 = cevent[1]
        if handle[ep1] then
            task.num = handle[ep1](obj, cevent) or 0
            task_check_num(task_obj, task, cevent, false)
        end

        if task.status ~= enums.taskstatus_finish then
            local marks = task_obj.marks
            local task_mark = gen_event_mark(cevent)
            marks[task_mark] = marks[task_mark] or {}
            marks[task_mark][taskid] = 1
        end
        task_obj.tasks[taskid] = task
    end

    ret.init_task = function(obj, task_obj, task_cfg)
        task_obj.marks = task_obj.marks or {}
        task_obj.tasks = task_obj.tasks or {}

        for taskid, tcfg in pairs(task_cfg) do
            init_one_task(obj, task_obj, taskid, tcfg)
        end
    end

    local count_one_task = function(task_obj, task, add_num, obj, cevent)
        print("count taskid", task.id)
        if task.status ~= enums.taskstatus_unfinish then
            return
        end
        local ep1 = cevent[1]
        local handle = init_handle
        if handle[ep1] then
            local ret = handle[ep1](obj, cevent)
            if ret then
                task.num = ret
            else
                task.num = task.num + add_num
            end
        end

        task_check_num(task_obj, task, cevent, true)
    end

    ret.count_task = function(task_obj, task_cfg, obj, tevent)
        local finish_tasks = {}

        local marks = task_obj.marks
        local task_mark = gen_event_mark(tevent)
        if not marks[task_mark] then
            return
        end
        local add_num = tevent[#tevent]
        for taskid, _ in pairs(marks[task_mark]) do
            local task = task_obj.tasks[taskid]
            local cevent = task_cfg[taskid].event
            count_one_task(task_obj, task, add_num, obj, cevent)
            if task.status == enums.taskstatus_finish then
                table.insert(finish_tasks, taskid)
            end
        end

        return finish_tasks
    end

    ret.add_task_mgr = function(name, task_mgr)
        task_mgrs[name] = task_mgr
    end
    ret.remove_task_mgr = function(name)
        task_mgrs[name] = nil
    end
    ret.trigger_event = function(obj, tevent)
        for name, task_mgr in pairs(task_mgrs) do
            task_mgr.trigger_event(obj, tevent)
        end
    end
    return ret
end

local init_handle = {
    [enums.tasktarg_build] = function(player, cfg_event)
        local build_type = cfg_event[2]
        if build_type == enums.build_main then
            return player.level
        end
    end
}
local player_task_mgrs = create_task_mgrs(init_handle)

-- task1_mgr test
local task1_mgr = {}
task1_mgr.trigger_event = function(player, tevent)
    local task = player.task1
    local finish_tasks = player_task_mgrs.count_task(task, task1_cfg, player, tevent)
    print_v(finish_tasks, "finish_tasks")
end
player_task_mgrs.add_task_mgr(enums.task1, task1_mgr)
-- player_task_mgrs.remove_task_mgr(enums.task1)

local player = {
    level = 5,
    task1 = {}
}
player_task_mgrs.init_task(player, player.task1, task1_cfg)
print_v(player, "player")
player.level = 10
player_task_mgrs.trigger_event(player, {enums.tasktarg_build, enums.build_main, 10})
print_v(player)
