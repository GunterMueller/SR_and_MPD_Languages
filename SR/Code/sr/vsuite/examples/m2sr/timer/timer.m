/* interval timer using a covering condition */
_monitor(timer)
  op delay(interval : int), tick()
_body(timer)

  var tod := 0
  _condvar(check)

  _proc( delay(interval) )
    var wake_time : int
    wake_time := tod + interval
    do wake_time > tod -> _wait(check) od
  _proc_end

  _proc( tick() )
    tod++
    _signal_all(check)
  _proc_end

_monitor_end
