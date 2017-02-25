(module
  (func (export "add") (param $x i32) (param $y i32) (result i32)
    get_local $x
    get_local $y
    i32.add)
)
(assert_return (invoke "add" (i32.const 10) (i32.const 20)) (i32.const 30))
