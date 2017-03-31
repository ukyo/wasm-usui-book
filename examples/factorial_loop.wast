(module
  (func $factorial_loop (export "factorial_loop") (param $n i32) (result i32)
    (local $ret i32)
    i32.const 1
    set_local $ret
    block $exit
      loop $cont
        get_local $n
        i32.eqz
        br_if $exit
        get_local $ret
        get_local $n
        i32.mul
        set_local $ret
        get_local $n
        i32.const 1
        i32.sub
        set_local $n
        br $cont
      end
    end
    get_local $ret)
)
(assert_return (invoke "factorial_loop" (i32.const 5)) (i32.const 120))
