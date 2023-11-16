_monitor(foo)
    op enter(), xexit()
_body(foo)
    var free := true
    _condvar(not_busy)
    _proc(enter())
        if not free -> _wait(not_busy) fi
        free := false
    _proc_end
    _proc(xexit())
        free := true
        _signal(not_busy)
    _proc_end
_monitor_end
