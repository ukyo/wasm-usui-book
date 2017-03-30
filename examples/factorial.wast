(module
  (func $factorial (export "factorial") (param $n i32) (result i32)
    get_local $n
    i32.eqz
    if i32
      i32.const 1
    else
      get_local $n
      get_local $n
      i32.const 1
      i32.sub
      call $factorial
      i32.mul
    end)
)
