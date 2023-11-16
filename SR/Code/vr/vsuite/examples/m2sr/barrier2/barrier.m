/* n process barrier -- awaken all waiting using _signal_all */
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
    if cnt = 0 ->
	if not _empty(b) -> write("oops1"); stop  fi
        _signal_all(b)
	if not _empty(b) -> write("oops2"); stop  fi
    fi
    cnt++
    /* split write statement so that sorted output is
     * nondeterministic.
     * (a weaker test for sure, but this is the only test
     * exercising signal_all.  other tests exercise rest of stuff.)
     */
    write("process", i)
    write("has arrived -- total of", cnt)
    if cnt < limit -> _wait(b)
    [] else ->
	if cnt != limit or _empty(b) -> write("oops3"); stop  fi
        _signal_all(b)
	if not _empty(b) -> write("oops4"); stop  fi
	cnt := 0
    fi
  _proc_end

_monitor_end
