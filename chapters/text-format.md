# Text Format

text formatに関してはS式形式でfixなのかを調査

## type

型宣言をします。現状は関数型のみ。

```
(module
  (type (func)) ;; 引数が0個で返り値がない関数
  (type $label (func)) ;; ラベルを付けることができる
  (type $type1 (func (param i32) (result i32))) ;; i32型の引数を一つ受け取り、i32型の値を返す
  (type $type2 (func (param i32 i64) (result f64))) ;; i32型の引数を一つ、i64型の引数を1つ受け取り、f64型の値を返す
)
```

## func

関数宣言とコード部分の定義を行います。

```
(module
  (func) ;; 何もしない関数
  (func $nop) ;; ラベル付けられる
  (func (param i32)) ;; i32型の引数を1つ受け取る
  (func (param i32 i64)) ;; i32型、i64型の引数を受け取る
  (func (param $x i32)) ;; 引数にラベル付けてみる
  (func (param $x i32) (param $y i64)) ;; 複数の場合はparamを分けて書く
  (func (param i32) (result i32) ;; i32型の引数を1つ受け取り、i32型の値を返す
    get_local 0) ;; 引数を0始まりのindexでよみこむ
  (func (param $x i32) (result i32)
    get_local $x) ;; かわりにラベルを使うことができる
  (func (param i32) (result i32)
    (local i32) ;; ローカル変数
    (local i32)
    i32.const 1
    set_local 1 ;; indexは引数を含んでいる
    i32.const 2
    tee_local 2
    get_local 0
    i32.add)
  (func (param $x i32) (result i32)
    (local $y i32) ;; ラベル使えます
    (local $z i32)
    i32.const 1
    set_local $y
    i32.const 2
    tee_local $z
    get_local 0
    i32.add)
  (func (type $type1) ;; 定義した型を参照する
    get_local 0)
)
```

## start

main関数的な奴

```
(module
  (func $foo)
  (start $foo) ;; function indexを指定する
)
```

## table, elem

何かしらの要素を置くための配列。
現状、tableはmodule内に1つまで置け、
要素の型はanyfunc(実際のデータはfunction index)だけが使える。
関数テーブルとして使用する。

```
(module
  (table 10 anyfunc) ;; 10個の領域を確保
)
(module
  (table 10 20 anyfunc) ;; 初期で10個、最大で20個。いまのところtableを動的に拡張できないっぽい?
)
(module
  (table 10 anyfunc)
  (elem (i32.const 0) $a) ;; initializerで指定したoffsetにfunction indexを流し込む
  (elem (i32.const 1) $a $b) ;; offset=1から順番に流し込む
  (func $a)
  (func $b)
)
(module
  (table anyfunc (elem $a $b $c)) ;; 略記法tableの大きさは3
  (func $a)
  (func $b)
  (func $c)
)
(module
  (type $foo (func (param i32 i32) (result i32)))
  (type $bar (func (param i32) (result i32)))
  (table anyfunc (elem $add $sub $mul $add2 $sub2))
  ;; 適当に関数を用意
  (func $add (type $foo) get_local 0 get_local 1 i32.add)
  (func $sub (type $foo) get_local 0 get_local 1 i32.sub)
  (func $mul (type $foo) get_local 0 get_local 1 i32.mul)
  (func $add2 (type $bar) get_local 0 i32.const 2 i32.add)
  (func $sub2 (type $bar) get_local 0 i32.const 2 i32.sub)

  (func $call_foo (param $i i32) (result i32)
    i32.const 10 i32.const 5 ;; 引数2個で
    get_local $i ;; $i番目の要素で指定されている関数を
    call_indirect $foo) ;; 型が$fooであると期待して呼ぶ
  
  (func $call_bar (param $i i32) (result i32)
    i32.const 4 ;; 引数1個で
    get_local $i ;; $i番目の要素で指定されている関数を
    call_indirect $bar) ;; 型が$barであると期待して呼ぶ
)
```

## memory, data

リニアーなメモリー領域を確保。ArrayBuffer的なやつ。
memoryはmoduleに1個まで。

```
(module (memory 10)) ;; 10 * 64KBでメモリー確保
(module (memory 10 20)) ;; 初期値で10 * 64KB。必要になったら最大20 * 64KBまで領域を拡張できる
(module
  (memory $memory 10)
  (data (i32.const 0) "abc\01\ffあいう") ;; initializerで指定したoffsetからUTF8でエンコーディングされたバイト列としてデータを流し込む
  (data (i32.const 128) "iiiii") ;; dataは複数書ける
) 
(module (memory (data "abc")) ;; 略記法、必要な分だけ確保してバイト列を先頭から突っ込む

```

## global

## import

## export
