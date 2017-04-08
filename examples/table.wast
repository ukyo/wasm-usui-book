(module
  (type $return_i32 (func (result i32)))
  (table (export "tbl") anyfunc (elem $f1 $f2 $f3))

  (func $f1 (result i32) i32.const 111)
  (func $f2 (result i32) i32.const 222)
  (func $f3 (result i32) i32.const 333)

  (func (export "call_indirect") (param $i i32) (result i32)
    get_local $i
    call_indirect $return_i32)
)
