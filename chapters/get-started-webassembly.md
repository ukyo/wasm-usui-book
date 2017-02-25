# Get Started WebAssembly

とりあえず足し算をする関数を持つwasmモジュールを実装してみましょう。バイナリを直接編集とかできないことはないのですが、ここではwasmのテキストフォーマットを使用します。

```
(module
  (func (export "add") (param $x i32) (param $y i32) (result i32)
    get_local $x
    get_local $y
    i32.add)
)
```

`(func ...)`は関数の定義部分で、
`(export "add")`はエクスポートされたときのメソッド名として使われます。
`(param $x i32) (pram $y i32) (result i32)`は関数の型を表します。ここでは32bit整数の引数を2個受け取り、32bit整数を返すということを表しています。
`get_local $x ...`からは関数のコード部分です。いわゆるスタックマシンです。`get_local $x`、`get_local $y`でローカル変数`$x`、`$y`(引数はローカル変数に含まれます)をスタックにpushして、`i32.add`でスタックから2個の値をpopして足した結果を返しています。これをadd.wasとして保存して、wast2wasmを使用してwasmモジュールのバイナリフォーマットにコンパイルします。

```
$ wast2wasm add.was -o add.wasm
```

wasmモジュールのバイナリフォーマットに対してWebAssemblyオブジェクトが持つメソッドを用いることで実行可能な関数等を含むインスタンスを生成することができます。

```js
fetch("add.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(obj => {
  console.log(obj.instance.exports.add(1, 2)); // 3
});
```

`add.wasm`(wasmのバイナリフォーマット)をfetch関数を用いてロードし、ファイル(ArrayBuffer)を`WebAssembly.instantiate`でインスタンス化します(インスタンス化されたモジュールを含むオブジェクトがPromiseとして返されます)。エクスポートされた関数は`instance.exports`に置かれます。

つぎにWebAssemblyでは外部から関数等をインポートする仕組みがあります。例としてprint関数をインポートしてみましょう。

```
(module
  (import "foo" "print" (func $print (param i32)))

  (func (export "bar") (param $x i32)
    get_local $x
    call $print)
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

`WebAssembly.instantiate`の第2引数にインポートするオブジェクトを指定します。`importObj.foo.print`で引数を1個受け取り、`console.log`で呼ぶメソッドが定義されています。これはWebAssemblyのテキスト表現`(import "foo" "print" (func $print (param i32)))`に対応していて、`call $print`で実際にそれが呼び出されます。

## この章のまとめ

* `WebAssembly.instantiate`でWebAssemblyモジュールのインスタンス化ができる
* エクスポートされた色々はインスタンス化されたモジュールの`exports`プロパティから見れる
* 外部の関数等をモジュールにインポートする仕組みがある
