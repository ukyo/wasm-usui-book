# Get Started WebAssembly

この章ではWebAssemblyのテキスト表現で簡単な例を書いてみて、WebAssemblyにはどんな機能があるのかをつかんでもらおうかと思います。

## 足し算してみる

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
  (func (export "add_each") (param $x i32) (param $n i32)
    (local $i i32)
    i32.const 0
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
        get_local $i
        i32.const 4
        i32.mul
        i32.load
        get_local $x
        i32.add
        i32.store
        get_local $i
        i32.const 1
        i32.add
        set_local $i
        br $cont
      end
    end)
)
```

## 関数テーブル

wip

## この章のまとめ

* `WebAssembly.instantiate`でWebAssemblyモジュールのインスタンス化ができる
* エクスポートされた色々はインスタンス化されたモジュールの`exports`プロパティから見れる
* 外部のモジュールをインポートする仕組みがある
* 連続したメモリ領域を使用できる
* 関数テーブルが使える
