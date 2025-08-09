return {
    [10] = {
        start_param = {
            time = {
                year = 2023,
                month = 1,
                day = 1,
                hour = 0
            }
        },
        duration = {
            day = 3
        }
    },
    [20] = {
        start_param = {
            start_server_after_day = 2
        },
        duration = {
            day = 1
        },
        next = {
            day = 6
        }
    },
    [30] = {
        start_param = {
            start_server_after_week = {
                week = 1,
                week_day = 1
            }
        },
        duration = {
            day = 1
        }
    },
    [40] = {
        start_param = {
            every_week = {2, 5}
        },
        duration = {
            day = 1
        }
    }
};
