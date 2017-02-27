# Get Started WebAssembly

この章ではWebAssemblyのテキスト表現で簡単な例を書いてみて、WebAssemblyにはどんな機能があるのかをつかんでもらおうかと思います。

## 足し算をしてみる

いわゆるHello World的なやつです。WebAssemblyのモジュールに整数の足し算を行う関数を定義して、JavaScriptから使えるようにしてみます。

```
;; ※これは行コメント
(module ;; (module ...)までが1個のモジュールになる
  (func ;; (func ...)が関数の定義部分
    (export "add") ;; この関数がモジュール外に`add`という名前でエクスポートされる
    (param $x i32) (param $y i32) (result i32) ;; 関数の型定義(後述)
    ;; 以下コード本体(後述)
    get_local $x
    get_local $y
    i32.add)
)
```

`(param $x i32) (pram $y i32) (result i32)`は関数の型を表しています。`(param $x i32)`は1個の32bit整数の引数を表し、`$x`というラベルを使って関数内から参照することができます。`(param $y i32)`は2個目の引数です。`(result i32)`は32bit整数の値を返すことを表します。
つぎに、`get_local $x ...`からは関数のコード本体部分です。これはいわゆるスタックマシンで、`get_local $x`、`get_local $y`でローカル変数`$x`、`$y`(引数はローカル変数に含まれます)をスタックにpushして、`i32.add`でスタックから2個の値をpopして足した結果を返しています。

これをadd.wasとして保存して、wast2wasmまたはvscode-wastのビルド機能を使用してwasmにコンパイルしてみましょう。

```
$ wast2wasm add.was -o add.wasm
```

JavaScript API用いて実行してみます。

```js
fetch("add.wasm")
.then(res => res.arrayBuffer()) // ArrayBufferとしてロード
.then(buffer => WebAssembly.instantiate(buffer)) // インスタンス化
.then(obj => {
  // instanceがインスタンスでexportsにエクスポートされたものが入ってきます
  console.log(obj.instance.exports.add(1, 2)); // 3
});
```

いまのところWebAssemblyのモジュールをJavaScriptの環境で使うためにはWebAssemblyのJavaScript APIで直接コンパイル、インスタンス化する必要があります。

## インポートしてみる

簡単なprint関数を作ってみましょう。とは言ってもWebAssemblyにprint関数的なものはないので外部からインポートする必要があります。

```
(module
  (import "foo" "print" (func $print (param i32))) ;; インポートする関数の型を記述

  (func (export "bar") (param $x i32)
    get_local $x
    call $print) ;; $print関数の呼び出し
)
```

```js
const importObj = {
  foo: {
    print: (x) => console.log(x)
  }
};

fetch("import.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer, importObj))
.then(obj => {
  obj.instance.exports.bar(1); // importObj.foo.printが呼ばれます
});
```

外部からモジュールをインポートする場合、`WebAssembly.instantiate`の第2引数にインポートするオブジェクトを指定します。この例の場合、オブジェクトの構造とWebAssemblyのテキスト表現`(import "foo" "print" (func $print (param i32)))`が対応していて、`call $print`で実際に呼ばれたことが確認できたと思います。

## 線形メモリーにアクセスしてみる

データの格納場所として線形メモリー(ただのバイト列みたいなもの)を使うことができます。

```
(module
  (memory (export "mem") 1)
  (func (export "add_each") (param $x i32) (param $offset i32) (param $n i32)
    (local $i i32)
    (local $ptr i32)
    i32.const 0
    set_local $i
    get_local $offset
    i32.const 4
    i32.mul
    set_local $ptr
    block $break
      loop $continue
        ;; ループのブレーク判定
        get_local $i
        get_local $n
        i32.eq
        br_if $break
        ;; メモリーの値を更新
        get_local $ptr
        get_local $ptr
        i32.load
        get_local $x
        i32.add
        i32.store
        ;; インクリメントしてループ先頭に飛ぶ
        get_local $i
        i32.const 1
        i32.add
        set_local $i
        get_local $ptr
        i32.const 4
        i32.add
        set_local $ptr
        br $continue
      end
    end)
)
```

```js
fetch("memory.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(obj => {
  const array = new Uint32Array(obj.instance.exports.mem.buffer);
  obj.instance.exports.add_each(10, 0, 5);
  console.log(array); // [10, 10, 10, 10, 10, 0, 0, ...]
  obj.instance.exports.add_each(30, 3, 3);
  console.log(array); // [10, 10, 10, 40, 40, 30, 0, ...]
});
```

## if文的なことをしてみる

## switch文的なことをしてみる

blockとbr_tableを組み合わせたらswitch文が!なお、可読性は低い。

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

```js
fetch("switch.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(obj => {
  console.log(obj.instance.exports.switch(0)); // 111
  console.log(obj.instance.exports.switch(1)); // 222
  console.log(obj.instance.exports.switch(2)); // 333
});
```

## 関数テーブルをつかってみる

関数テーブルで関数を間接参照して呼び出せます。switch文よりは可読性高い。
テーブルの仕組みはもうちょっと汎用性が高い配列といった感じだが、今のところ関数のみ有効。

```
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
```

```js
fetch("table.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(obj => {
  console.log(obj.instance.exports.call_indirect(0)); // 111
  console.log(obj.instance.exports.call_indirect(1)); // 222
  console.log(obj.instance.exports.call_indirect(2)); // 333

  // エクスポートしたtableはJSから使えるぞ!
  console.log(obj.instance.exports.tbl.get(0)()); // 111
  console.log(obj.instance.exports.tbl.get(1)()); // 222
  console.log(obj.instance.exports.tbl.get(2)()); // 333
});
```

## この章のまとめ

* `WebAssembly.instantiate`でWebAssemblyモジュールのインスタンス化ができる
* エクスポートされた色々はインスタンス化されたモジュールの`exports`プロパティから見れる
* 外部のモジュールをインポートする仕組みがある
* 連続したメモリ領域を使用できる
* 一通りの制御構文がつかえる
* 関数テーブルが使える
