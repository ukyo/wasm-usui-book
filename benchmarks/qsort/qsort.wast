(module
  (import "js" "mem" (memory 256))

  (func $med3_f64 (param $x f64) (param $y f64) (param $z f64) (result f64)
    get_local $x
    get_local $y
    f64.lt
    if f64
      get_local $y
      get_local $z
      f64.lt
      if
        get_local $y
        return
      end
      get_local $x
      get_local $z
      get_local $z
      get_local $x
      f64.lt
      select
    else
      get_local $z
      get_local $y
      f64.lt
      if
        get_local $y
        return
      end
      get_local $x
      get_local $z
      get_local $x
      get_local $z
      f64.lt
      select
    end)
  
  (func $med3_i32 (param $x i32) (param $y i32) (param $z i32) (result i32)
    get_local $x
    get_local $y
    i32.lt_s
    if i32
      get_local $y
      get_local $z
      i32.lt_s
      if
        get_local $y
        return
      end
      get_local $x
      get_local $z
      get_local $z
      get_local $x
      i32.lt_s
      select
    else
      get_local $z
      get_local $y
      i32.lt_s
      if
        get_local $y
        return
      end
      get_local $x
      get_local $z
      get_local $x
      get_local $z
      i32.lt_s
      select
    end)
  
  (func $qsort_f64 (param $left i32) (param $right i32)
    (local $i i32)
    (local $j i32)
    (local $pivot f64)
    
    get_local $left
    get_local $right
    i32.lt_s
    if
      get_local $left
      tee_local $i
      f64.load
      get_local $right
      tee_local $j
      f64.load
      get_local $i
      get_local $j
      get_local $i
      i32.sub
      i32.const 1
      i32.shr_u
      i32.add
      i32.const 0xfffffff8
      i32.and
      f64.load
      call $med3_f64
      set_local $pivot
      loop $cont
        loop
          get_local $i
          f64.load
          get_local $pivot
          f64.lt
          if
            get_local $i
            i32.const 8
            i32.add
            set_local $i
            br 1
          end
        end
        loop
          get_local $pivot
          get_local $j
          f64.load
          f64.lt
          if
            get_local $j
            i32.const 8
            i32.sub
            set_local $j
            br 1
          end
        end
        get_local $i
        get_local $j
        i32.lt_s
        if
          get_local $j
          get_local $i
          f64.load
          get_local $i
          get_local $j
          f64.load
          f64.store
          f64.store
          get_local $i
          i32.const 8
          i32.add
          set_local $i
          get_local $j
          i32.const 8
          i32.sub
          set_local $j
          br $cont
        end
      end
      get_local $left
      get_local $i
      i32.const 8
      i32.sub
      call $qsort_f64
      get_local $j
      i32.const 8
      i32.add
      get_local $right
      call $qsort_f64
    end)

  (func (export "qsort_f64") (param i32 i32)
    get_local 0
    i32.const 3
    i32.shl
    get_local 1
    i32.const 1
    i32.sub
    i32.const 3
    i32.shl
    call $qsort_f64)

  (func $qsort_i32 (param $left i32) (param $right i32)
    (local $i i32)
    (local $j i32)
    (local $pivot i32)
    
    get_local $left
    get_local $right
    i32.lt_s
    if
      get_local $left
      tee_local $i
      i32.load
      get_local $right
      tee_local $j
      i32.load
      get_local $i
      get_local $j
      get_local $i
      i32.sub
      i32.const 1
      i32.shr_u
      i32.add
      i32.const 0xfffffffc
      i32.and
      i32.load
      call $med3_i32
      set_local $pivot
      loop $cont
        loop
          get_local $i
          i32.load
          get_local $pivot
          i32.lt_s
          if
            get_local $i
            i32.const 4
            i32.add
            set_local $i
            br 1
          end
        end
        loop
          get_local $pivot
          get_local $j
          i32.load
          i32.lt_s
          if
            get_local $j
            i32.const 4
            i32.sub
            set_local $j
            br 1
          end
        end
        get_local $i
        get_local $j
        i32.lt_s
        if
          get_local $j
          get_local $i
          i32.load
          get_local $i
          get_local $j
          i32.load
          i32.store
          i32.store
          get_local $i
          i32.const 4
          i32.add
          set_local $i
          get_local $j
          i32.const 4
          i32.sub
          set_local $j
          br $cont
        end
      end
      get_local $left
      get_local $i
      i32.const 4
      i32.sub
      call $qsort_i32
      get_local $j
      i32.const 4
      i32.add
      get_local $right
      call $qsort_i32
    end)

  (func (export "qsort_i32") (param i32 i32)
    get_local 0
    i32.const 2
    i32.shl
    get_local 1
    i32.const 1
    i32.sub
    i32.const 2
    i32.shl
    call $qsort_i32)
)
