(module
  (global $count (mut i32) (i32.const 0))

  (func (export "increment") (result i32)
    get_global $count
    i32.const 1
    i32.add
    set_global $count
    get_global $count)
)
(assert_return (invoke "increment") (i32.const 1))
(assert_return (invoke "increment") (i32.const 2))
(assert_return (invoke "increment") (i32.const 3))
