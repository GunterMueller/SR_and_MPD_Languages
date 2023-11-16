/* CSCAN disk scheduler problem
 * like other cscan test, but forces reproducibility.
 * tests: nested monitor calls (exclusion in first not released)
 * _pri_wait, _minrank, _empty, _print,
 * arrays of condition variables,
 * two monitors in a file.
 */


_monitor(repro)
    op entry(pid: int), increment()
_body(repro)
    /* for reproducibility:
     * order[turn] is pid of process allowed to enter monitor next.
     * hangout[pid] is private condition variable for process pid
     * on which it waits until its turn.
     * assume we know in advance number of processes.
     */
    var turn := 1
    /* output will be the same as the releases below. */
    var order[19] := ( 1, /* requests 40 */
                       1, /* releases */
                       3, /* requests 30 */
                       2, /* requests 35 */
                       1, /* requests 20 */
                       3, /* releases */
                       3, /* requests 50 */
                       2, /* releases */
                       2, /* requests 60 */
                       3, /* releases */
                       2, /* releases */
                       2, /* requests 10 */
                       3, /* requests 5 */
                       1, /* releases */
                       3, /* releases */
                       2, /* releases */
                       2, /* requests 80 */
                       2, /* releases */
                       1) /* extra to simplify termination */
                          /* 1, 2, or 3 will work. */

    _condvar1(hangout, 3 )

    _proc( entry(pid) )
        if order[turn] != pid -> _wait(hangout[pid]) fi
    _proc_end

    _proc( increment() )
        turn++
        _signal(hangout[order[turn]])
    _proc_end
_monitor_end


_monitor(cscan)
    op request(cyli: int), release()
_body(cscan)
    import repro
    _condvar1(scan, 0:1 )
    var c := 0, n := 1, pos := -1

    _proc( request(cyli) )
        repro.increment()
        if pos = -1 ->
            pos := cyli
        [] pos != -1  and cyli > pos ->
             _pri_wait(scan[c],cyli)
        [] pos != -1  and cyli <= pos ->
             _pri_wait(scan[n],cyli)
        fi
    _proc_end

    _proc( release() )
        repro.increment()
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
