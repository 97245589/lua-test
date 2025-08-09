require "util"

local enums = {
    unfinish = 1,
    finish = 2,

    player_level = 1,
    build_level = 2,

    build_main = 1,
    build_room = 2
}

local task_cfg = {
    [1] = {
        event = {1, 3}
    },
    [2] = {
        event = {1, 5}
    },
    [20] = {
        event = {2, 1, 10}
    }
}

local create_task_mgrs = function(handle)
    local gen_event_mark = function(event_arr)
        return table.concat(event_arr, "|", 1, #event_arr - 1)
    end

    local task_mgrs = {}
    local M = {}

    local task_count = function(obj, task, tevent, add_num)
        local ep1 = tevent[1]
        if handle[ep1] then
            task.num = handle[ep1](obj, tevent) or 0
        else
            task.num = task.num + add_num
        end

        local num = tevent[#tevent]
        if task.num >= num then
            task.num = num
            task.status = enums.finish
            return true
        end
    end

    local add_event_mark = function(marks, event_mark, taskid)
        marks[event_mark] = marks[event_mark] or {}
        marks[event_mark][taskid] = 1
    end

    local remove_event_mark = function(marks, event_mark, taskid)
        marks[event_mark][taskid] = nil
        if not next(marks[event_mark]) then
            marks[event_mark] = nil
        end
    end

    local init_one_task = function(obj, task_obj, taskid, tevent)
        local tasks = task_obj.tasks
        if tasks[taskid] then
            return
        end
        local task = {
            id = taskid,
            num = 0,
            status = enums.unfinish
        }
        task_count(obj, task, tevent, 0)

        if task.status == enums.unfinish then
            local event_mark = gen_event_mark(tevent)
            add_event_mark(task_obj.marks, event_mark, taskid)
        end
        task_obj.tasks[taskid] = task
    end
    M.init_task = function(obj, task_obj, task_cfg)
        task_obj.marks = task_obj.marks or {}
        task_obj.tasks = task_obj.tasks or {}

        for taskid, tcfg in pairs(task_cfg) do
            init_one_task(obj, task_obj, taskid, tcfg.event)
        end
    end

    M.count_task = function(obj, task_obj, task_cfg, event)
        local event_mark = gen_event_mark(event)
        local taskids = task_obj.marks[event_mark]
        if not taskids then
            print("no taskids ---", event_mark, dump(event))
            return
        end

        local t = {}
        local add_num = event[#event]
        for taskid, _ in pairs(taskids) do
            local tevent = task_cfg[taskid].event
            local task = task_obj.tasks[taskid]
            task_count(obj, task, tevent, add_num)
            if task.status ~= enums.unfinish then
                remove_event_mark(task_obj.marks, event_mark, task.id)
            end
            table.insert(t, taskid)
        end
        return next(t) and t
    end

    M.add_taskmgr = function(name, mgr)
        task_mgrs[name] = mgr
    end
    M.trigger_event = function(obj, pevent)
        for name, mgr in pairs(task_mgrs) do
            mgr.trigger_event(obj, pevent)
        end
    end
    return M
end

local mgrs = create_task_mgrs({
    [enums.player_level] = function(player, tevent)
        return player.level
    end
})

local player = {
    level = 1,
    task = {}
}

local task_mgr = {
    trigger_event = function(player, pevent)
        local taskids = mgrs.count_task(player, player.task, task_cfg, pevent)
        print("change taskids", dump(taskids))
    end
}
mgrs.add_taskmgr("test", task_mgr)
mgrs.init_task(player, player.task, task_cfg)
print("player after init", dump(player))
player.level = 10
mgrs.trigger_event(player, {enums.player_level, 10})
mgrs.trigger_event(player, {enums.build_level, enums.build_main, 5})
mgrs.trigger_event(player, {enums.build_level, enums.build_main, 6})
mgrs.trigger_event(player, {enums.build_level, enums.build_room, 5})
mgrs.trigger_event(player, {10, 3})
mgrs.trigger_event(player, {5, 5, 5})
print("after trigger", dump(player))
