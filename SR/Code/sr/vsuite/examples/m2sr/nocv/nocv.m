/* monitor that uses no condition variables */
_monitor(no_cv)
  op setv(n: int), getv() returns n: int
_body(no_cv)
  var value := 0
  _proc( setv(n) )
    value := n
  _proc_end

  _proc( getv() returns n )
    n := value
  _proc_end

_monitor_end
