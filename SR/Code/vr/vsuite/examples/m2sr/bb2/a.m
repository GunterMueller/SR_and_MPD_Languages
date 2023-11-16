_monitor(foo)
    op enter(who:int), xexit()
_body(foo)
    var free := true
    _condvar(not_busy)
    _proc(enter(who))
        if not free -> _pri_wait(not_busy,who) fi
        free := false
    _proc_end
    _proc(xexit())
        if _empty(not_busy) -> write("not_busy empty")
        [] else -> write("not_busy minrank is", _minrank(not_busy))
	fi
	_print(not_busy)
        free := true
        _signal(not_busy)
    _proc_end
_monitor_end
