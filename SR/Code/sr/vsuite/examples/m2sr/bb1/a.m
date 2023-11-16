/* use only this form of commenting, but don't nest */
_monitor(bounded_buffer)
    op deposit(data:int), fetch() returns data:int
_body(bounded_buffer)
    const N := 4
    var buf[0:N-1]: int
    var front := 0, rear := 0, count := 0
    _condvar(not_full); _condvar(not_empty)
    _proc( deposit(data) )
        do count = N -> # do (not if) to handle signal stealers in SC
            _print(not_full); _print(not_empty)
            _wait(not_full)
        od
        _print(not_full); _print(not_empty)
        buf[rear] := data
        rear := (rear+1) % N
        count := count+1
        _signal(not_empty)
    _proc_end
    _proc( fetch() returns result )
        do count = 0 -> # do (not if) to handle signal stealers in SC
            _print(not_full); _print(not_empty)
            _wait(not_empty)
        od
        _print(not_full); _print(not_empty)
        result := buf[front]
        front := (front+1) % N
        count := count-1
        _signal(not_full)
    _proc_end
_monitor_end
