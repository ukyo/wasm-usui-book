(module
  (func $f (param i32))
  (global $x i32 (i32.const 100))
  (memory $mem 2)
  (table $tbl anyfunc (elem $f))

  (export "f" (func $f))
  (export "x" (global $x))
  (export "mem" (memory $mem)) ;; 現状memoryは最大1個なので基本的に(memory 0)でOK
  (export "tbl" (table $tbl))  ;; 上と同じ理由で(table 0)でOK
)
