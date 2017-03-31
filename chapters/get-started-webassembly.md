# Get Started WebAssembly

この章ではwastで簡単な例を書いてみて、WebAssemblyにどんな機能があるのかをつかんでもらおうかと思います。

## 関数を定義してみる

WebAssemblyのモジュールを作り、整数の足し算を行う関数を定義して、JavaScriptから使えるようにエクスポートします。

```
(module
  (func
    (export "add")
    (param $x i32) (param $y i32) (result i32)
    get_local $x
    get_local $y
    i32.add)
)
```

`(module ...)`でWebAssemblyモジュールを作ります。`(func ...)`で関数を定義して、次の行の`(export "add")`で`add`というプロパティ名でアクセスできるように関数をエクスポートしています。`(param $x i32) (pram $y i32) (result i32)`は関数の型を表しています。`(param $x i32)`は1個の32bit整数の引数を表し、`$x`というラベルを使って関数内から参照することができます。`(param $y i32)`は2個目の引数です。`(result i32)`は32bit整数の値を返すことを表します。
次に、`get_local $x ...`からは関数のコード本体部分で、これはスタックマシンの命令列です。`get_local $x`、`get_local $y`でローカル変数`$x`、`$y`（引数はローカル変数に含まれます）から値を読み込んでスタックにpushし、`i32.add`でスタックから2個の値をpopして足した結果をpushします。最後にスタックに積まれている値が関数の戻り値となります。

このコードのままではJSから読み込んで実行することはできないので、wasm（バイナリ形式のコード）に変換します。add.wastとして保存して、wast2wasm、またはvscode-wastのビルド機能を使用してwasmにします。

```
$ wast2wasm add.wast -o add.wasm
```

wasmファイルは`fetch`（もちろんXHRでも可）でロードし、JS APIを用いて実行可能な形式にコンパイル、インスタンス化して初めて実行することができます。

```js
fetch("add.wasm")
.then(res => res.arrayBuffer())                  // ArrayBufferとしてロード
.then(buffer => WebAssembly.instantiate(buffer)) // インスタンス化
.then(({module, instance}) => {
  // instance.exportsにエクスポートされたものが入ってきます
  console.log(instance.exports.add(1, 2));       // 3
});
```

## インポートしてみる

WebAssemblyはモジュール外部の関数をインポートすることができます。`console.log`をラップした関数をインポートとしてモジュール内部で使用してみます。

```
(module
  (import "foo" "print" (func $print (param i32)))
  (func (export "bar") (param $x i32)
    get_local $x
    call $print)
)
```

`(import "foo" "print" (func $print (param i32)))`がインポートの宣言です。`"foo"`はインポートするモジュール名、`"print"`がプロパティ名、`(func $print (param i32))`が関数の型を表します。インポートされた関数は`call $print`で呼ばれます。

```js
const importObj = {
  foo: {
    print: (x) => console.log(x)
  }
};

fetch("import.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer, importObj))
.then(({module, instance}) => {
  instance.exports.bar(1); // importObj.foo.printが呼ばれます
});
```

インポートする場合、`WebAssembly.instantiate`の第2引数にインポートするオブジェクトを指定します。この例の場合、オブジェクトの構造とwast内でのインポート宣言が対応しています。

## if文的なことをしてみる

いわゆるif文をWebAssemblyで実現するためにはifブロックを使用します。

```
(module
  (fn (export "if") (param $x i32) (result i32)
    get_local $x
    if i32
      i32.const 100
    end
    i32.const 10)

  (fn (export "if_then_else") (param $x i32) (result i32)
    get_local $x
    if i32
      i32.const 100
    else
      i32.const 10
    end)
)
```

`if i32`から`end`までがifブロックです。スタックからpopした値が0でない場合のみifブロックに入ります。`if`の直後の`i32`はブロックを抜けるときにpushする値の型です。何もpushしない場合は型の部分を空にしてください。もしelseブロックが存在する場合popした値が0のときにそちらに飛びます。

## switch文的なことをしてみる

`block`と`br_table`を組み合わせたらswitch文のようなことができます。

```
(module
  (func (export "switch") (param $i i32) (result i32)
    block $a
      block $b
        block $c
          get_local $i
          br_table $a $b $c
        end
        i32.const 333
        return
      end
      i32.const 222
      return
    end
    i32.const 111)
)
```

`block`は対応する`end`までのブロックを作ります。`br_table`はネストしたブロックに対してスタックからpopした値をインデックスとして分岐する先を指定します。`$a $b $c`が対象のブロックです。

```js
fetch("switch.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(({module, instance}) => {
  console.log(instance.exports.switch(0)); // 111
  console.log(instance.exports.switch(1)); // 222
  console.log(instance.exports.switch(2)); // 333
});
```

## 再帰してみる

関数はそれ自身を指定して呼び出すことができるので、再帰関数を実現できます。以下のfactorial関数は階乗を求める関数です。

```
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
```

## ループしてみる

ループを実現するためにはloopブロックを使います。上で定義したfactorial関数をループ版に書き直します。

```
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
```

`loop`と、それに対応する`end`までがloopブロックです。いわゆるwhile文のようなものを実装するときは、`block`と合わせて使うことが多いです。`loop`のすぐ後に出てくる`br_if $exit`で終了判定をし、真なら外側のブロックを抜けます。`loop`に対応する`end`直前の`br $cont`でloopブロックの先頭に飛びます。

## グローバル変数を使ってみる

モジュール内で共通してアクセスできるグローバル変数を定義することができます。実行する度に値をインクリメントする関数を作ってみます。

```
(module
  (global $count (mut i32) (i32.const 0))

  (func (export "increment") (result i32)
    get_global $count
    i32.const 1
    i32.add
    set_global $count
    get_global)
)
```

`(global $count (mut i32) (i32.const 0))`がグローバル変数の定義です。`(mut i32)`は変数のミュータビリティーと型を表します。`(mut hoge)`だとミュータブルになります。イミュータブルにしたい場合は単純に`i32`と書いてください。`(i32.const 0)`は変数のイニシャライザーです。定数や、他のグローバル変数を参照することができます。グローバル変数にアクセスするときは`get_global`、`set_global`を使用します。

```js
fetch("increment.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(({module, instance}) => {
  console.log(instance.exports.increment()); // 1
  console.log(instance.exports.increment()); // 2
  console.log(instance.exports.increment()); // 3
});
```

## 線形メモリーにアクセスしてみる

データの格納場所として線形メモリーを使うことができます。これは生のバイト列のようなもので、ロード、ストア命令によってモジュール内部から線形メモリにアクセスすることができます。以下のsum関数はn個の32bit整数を合計した値を返す関数です。

```
(module
  (memory (export "mem") 1 2)
  (data (i32.const 0) "\01\00\00\00\02\00\00\00\03\00\00\00")
  (func (export "sum") (param $n i32) (result i32)
    (local $i i32)
    (local $ret i32)
    i32.const 0
    tee_local $ret
    set_local $i
    block $exit
      loop $cont
        get_local $i
        get_local $n
        i32.eq
        br_if $exit
        get_local $i
        i32.const 4
        i32.mul
        i32.load
        get_local $ret
        i32.add
        set_local $ret
        get_local $i
        i32.const 1
        i32.add
        set_local $i
        br $cont
      end
    end
    get_local $ret)
)
```

`(memory (export "mem") 1 2)`がメモリーの定義です。`1 2`はメモリーの初期サイズと最大サイズを表します。メモリーを確保する最小単位は64KBで、つまり、初期サイズは64KB、最大サイズは128KBとなります。メモリーのサイズを拡張する場合は`grow_memory`命令、現在のサイズを取得したい場合は`current_memory`命令を使用します。`(data (i32.const 0) "\01\00\00\00\02\00\00\00\03\00\00\00")`でメモリーの初期化時にデータを書き込みます。`(i32.const 0)`はメモリーのオフセット、`"\01\00\00\00\02\00\00\00\03\00\00\00"`は挿入するバイト列を表します。`i32.load`命令を使用してメモリーからデータを読み込みます。その際、バイトオーダーはリトルエンディアンとして扱われます。

```js
fetch("sum.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(({module, instance}) => {
  const mem = instance.exports.mem;
  console.log(mem.buffer.byteLength);                    // 65536
  const array = new Uint32Array(mem.buffer);
  console.log(instance.exports.sum(3));                  // 6
  array.subarray(0, 10).forEach((x, i) => array[i] = i);
  console.log(instance.exports.sum(10));                 // 45
});
```

メモリーをエクスポートする場合、`WebAssembly.Memory`のインスタンスとしてエクスポートされます。生データはArrayBufferとして`buffer`プロパティからアクセスすることができます。

## 関数テーブルを使ってみる

WebAssemblyでは指定した型の要素の参照を持つテーブルを定義できます。現状指定できる型は関数だけで、これは関数テーブルとして利用できます。[switch文的なことをしてみる](#switch文的なことをしてみる)をテーブルを使って書き直します。

```
(module
  (type $return_i32 (func (result i32)))
  (table (export "tbl") anyfunc 3 4)
  (elem (i32.const 0) $f1 $f2 $f3)

  (func $f1 (result i32) i32.const 111)
  (func $f2 (result i32) i32.const 222)
  (func $f3 (result i32) i32.const 333)

  (func (export "call_indirect") (param $i i32) (result i32)
    get_local $i
    call_indirect $return_i32)
)
```

```js
fetch("table.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(({module, instance}) => {
  console.log(instance.exports.call_indirect(0)); // 111
  console.log(instance.exports.call_indirect(1)); // 222
  console.log(instance.exports.call_indirect(2)); // 333
  const tbl = instance.exports.tbl;
  console.log(tbl.get(0)()); // 111
  console.log(tbl.get(1)()); // 222
  console.log(tbl.get(2)()); // 333
});
```

## この章のまとめ

* `WebAssembly.instantiate`でWebAssemblyモジュールのインスタンス化ができる
* エクスポートされた色々はインスタンス化されたモジュールの`exports`プロパティから見れる
* 外部のモジュールをインポートする仕組みがある
* 連続したメモリ領域を使用できる
* 一通りの制御構文がつかえる
* 関数テーブルが使える
