/* n process barrier
 * weird behavior for SX:
 * works, but not as intended barrier; cnt increases...
 */
_monitor(barrier)
  op setn(n: int), arrive(i: int)
_body(barrier)

  _condvar(b)
  var cnt := 0, limit : int

  _proc( setn(n) )
    limit := n
    write("number of processes is", n)
  _proc_end

  _proc( arrive(i) )
    cnt++
    write("process", i, "has arrived -- total of", cnt)
    if cnt < limit -> _wait(b)
    [] else ->
        fa i := 1 to limit-1 -> _signal(b) af
        cnt := 0
    fi
  _proc_end

_monitor_end
