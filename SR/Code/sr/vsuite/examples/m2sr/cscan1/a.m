/* CSCAN disk scheduler */
_monitor(cscan)
    op request(cyli: int), release()
_body(cscan)
    _condvar1(scan, 0:1 )
    var c := 0, n := 1, pos := -1

    _proc( request(cyli) )
        if pos = -1 ->
            pos := cyli
        [] pos != -1  and cyli > pos ->
             _pri_wait(scan[c],cyli)
        [] pos != -1  and cyli <= pos ->
             _pri_wait(scan[n],cyli)
        fi
    _proc_end

    _proc( release() )
    	write("on release, current scan; next scan:")
	_print(scan[c]);	_print(scan[n])
        if  not  _empty(scan[c]) ->
            pos :=  _minrank(scan[c])
        []  _empty(scan[c])  and  not  _empty(scan[n]) ->
            c :=: n
            pos :=  _minrank(scan[c])
        []  _empty(scan[c])  and  _empty(scan[n]) ->
            pos := -1
        fi
        _signal(scan[c])
    _proc_end
_monitor_end
