# Text Format

## コメント

いわゆるコメントです。

```
;; 一行コメント

(;
ブロックコメント
;)
```

## ラベル

バイナリ表現では他のセクションのエントリーやローカル変数などを参照するときに直接インデックスを指定していましたがラベルを指定(例: `$foo`)することでインデックスの代わりにラベルで参照することができます。

## module

wasm module。1ファイル1moduleです。

```
(module
  ;; 各セクションのエントリーが入ります
)
```

## Sections

### type

型宣言をします。

```
(module
  (type (func))                                     ;; 引数が0個で返り値がない関数
  (type $label (func))                              ;; ラベル
  (type $type1 (func (param i32) (result i32)))     ;; i32型の引数を一つ受け取り、i32型の値を返す
  (type $type2 (func (param i32 i64) (result f64))) ;; i32型の引数を一つ、i64型の引数を1つ受け取り、f64型の値を返す
)
```

### func

関数宣言とコード本体の定義をセットで行います。

```
(module
  (func)                               ;; 何もしない関数
  (func $nop)                          ;; ラベル付けられる
  (func (param i32))                   ;; i32型の引数を1つ受け取る
  (func (param i32 i64))               ;; i32型、i64型の引数を受け取る
  (func (param $x i32))                ;; 引数にラベル付けてみる
  (func (param $x i32) (param $y i64)) ;; 複数の場合はparamを分けて書く
  (func (param i32) (result i32)       ;; i32型の引数を1つ受け取り、i32型の値を返す
    get_local 0)
  (func (param i32) (result i32)
    (local i32)                        ;; ローカル変数の宣言
    (local $a i32)                     ;; ラベル付き
    i32.const 1
    set_local 1
    i32.const 2
    tee_local $a
    get_local 0
    i32.add)
  (func (type $type1)                  ;; typeを参照する
    get_local 0)
)
```

### start

インスタンス化されたときに呼ばれるfuncを指定します。

```
(module
  (func $foo)
  (start $foo) ;; function indexかラベルで指定する
)
```

### table, elem

table(型付き配列)を定義します。elemでtableに要素を挿入します。tableは各moduleに対して1個以下です。

```
(module
  (table 10 anyfunc)              ;; 型がanyfuncで10個の領域を持つtableを作成
)

(module
  (table 10 20 anyfunc)           ;; 初期で10個、最大で20個まで拡張できるtableを作成(ちなみにwasm内から拡張できないです)
)

(module
  (table 10 anyfunc)
  (elem (i32.const 0) $a)         ;; initializerで指定したoffset(0)に要素を流し込む
  (elem (i32.const 1) $a $b)      ;; offset=1から順番に流し込む
  (func $a)
  (func $b)
)

(module
  (table anyfunc (elem $a $b $c)) ;; 略記法。要素数が3で、それぞれ$a $b $cのfunction indexを持つ要素が入る
  (func $a)
  (func $b)
  (func $c)
)

;; call_indirectの例
(module
  (type $foo (func (param i32 i32) (result i32)))
  (type $bar (func (param i32) (result i32)))
  (table anyfunc (elem $add $sub $mul $add2 $sub2))
  
  (func $add (type $foo) get_local 0 get_local 1 i32.add)
  (func $sub (type $foo) get_local 0 get_local 1 i32.sub)
  (func $mul (type $foo) get_local 0 get_local 1 i32.mul)
  (func $add2 (type $bar) get_local 0 i32.const 2 i32.add)
  (func $sub2 (type $bar) get_local 0 i32.const 2 i32.sub)

  (func $call_foo (param $i i32) (result i32)
    i32.const 10 i32.const 5                               ;; 引数2個で
    get_local $i                                           ;; tableの$i番目の要素から参照されている関数を
    call_indirect $foo)                                    ;; typeが$fooであると期待して呼ぶ
  
  (func $call_bar (param $i i32) (result i32)
    i32.const 4                                            ;; 引数1個で
    get_local $i                                           ;; tableの$i番目の要素から参照されている関数を
    call_indirect $bar)                                    ;; typeが$barであると期待して呼ぶ
)
```

### memory, data

memoryを作成します。memoryはmoduleに1個まで。dataでmemoryにバイト値を挿入します。

```
(module (memory 10))                     ;; 10ページ分(10 * 64KB)のメモリー確保
(module (memory 10 20))                  ;; 初期値で10ページ確保。必要になったら最大20ページまで領域を拡張できる
(module
  (memory $memory 10)                    ;; ラベル付き
  (data (i32.const 0) "abc\01\ffあいう") ;; initializerで指定したoffset(0byte目)からUTF8でエンコーディングされたバイト列としてデータを流し込む
  (data (i32.const 128) "iiiii")         ;; offset(128バイト目)から流し込む
) 
(module (memory (data "abc"))            ;; 略記法、必要な分だけ確保してバイト列を先頭から突っ込む

```

### global

グローバル変数を定義します。

```
(module
  (global i32 (i32.const 1))               ;; initializerの返却値を初期値としたイミュータブルなグローバル変数を定義
  (global $foo i64 (i64.const 100))        ;; ラベル付き
  (global $bar (mut f32) (f32.const 1.11)) ;; ミュータブルなグローバル変数
)
```

### import

### export

## コード本体

スタック記法とS式記法の2種類が使えるのでそれぞれのパターン書く

### 基本的なオペレータ

i32.addとか

### フロー系オペレータ

block, if, loop, br, br_if, br_table

### load, store系オペレータ

i32.load, i32.storeとか
