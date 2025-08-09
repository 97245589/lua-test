local tprint;
tprint = function(tb, name)
    local handle_key = function(k)
        if "string" == type(k) then
            return "[\"" .. k .. "\"]";
        elseif "number" == type(k) then
            return "[" .. k .. "]";
        else
            return "[[" .. type(k) .. "]]";
        end
    end

    local check = {};
    local tprint;
    tprint = function(tb, level, name)
        if type(tb) ~= "table" then
            print(tb);
            return;
        end
        name = name or "table";
        level = level or 0;

        local space = "    ";
        local start_line = handle_key(name) .. " = {";
        local allspace = "";
        for i = 1, level do
            allspace = allspace .. space;
        end
        if check[tb] then
            print(allspace .. handle_key(name) .. " = deadloop");
            return;
        else
            check[tb] = 1;
        end
        start_line = allspace .. start_line;
        print(start_line);

        for k, v in pairs(tb) do
            local str_val = space .. allspace;
            if "string" == type(k) then
                str_val = str_val .. "[\"" .. k .. "\"] = ";
            elseif "number" == type(k) then
                str_val = str_val .. "[" .. k .. "] = ";
            else
                str_val = str_val .. type(k) .. " = ";
            end

            if type(v) == "string" then
                str_val = str_val .. "\"" .. v .. "\",";
                print(str_val);
            elseif type(v) == "number" then
                str_val = str_val .. v .. ",";
                print(str_val);
            elseif type(v) == "table" then
                tprint(v, level + 1, k);
            else
                str_val = str_val .. type(v) .. ",";
                print(str_val);
            end
        end

        local end_line = allspace .. "},";
        print(end_line);
    end
    tprint(tb, 0, name);
end
print_table = tprint;