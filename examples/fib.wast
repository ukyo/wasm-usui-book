(module
  (func $fib (param $n i64) (result i64)
    get_local $n
    i64.const 1
    i64.le_u
    if i64
      get_local $n
    else
      get_local $n
      i64.const 1
      i64.sub
      call $fib
      get_local $n
      i64.const 2
      i64.sub
      call $fib
      i64.add
    end)
  
  (func (export "fib") (param $n f64) (result f64)
    get_local $n
    i64.trunc_u/f64
    call $fib
    f64.convert_u/i64)

  (func $fib_loop (export "fib_loop") (param $n i64) (result f64)
    (local $a i64)
    (local $b i64)
    i64.const 0
    set_local $a
    i64.const 1
    set_local $b
    block $exit
      loop $cont
        get_local $n
        i64.eqz
        br_if $exit
        get_local $a
        get_local $b
        i64.add
        get_local $b
        set_local $a
        set_local $b
        get_local $n
        i64.const 1
        i64.sub
        set_local $n
        br $cont
      end
    end
    get_local $a
    f64.convert_u/i64)
)
