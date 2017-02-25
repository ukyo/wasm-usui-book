(module
  (import "foo" "print" (func $print (param i32)))

  (func (export "bar") (param $x i32)
    get_local $x
    call $print)
)
