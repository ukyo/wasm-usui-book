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
.then(obj => {
  obj.instance.exports.bar(1); // importObj.foo.printが呼ばれます
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
.then(obj => {
  console.log(obj.instance.exports.switch(0)); // 111
  console.log(obj.instance.exports.switch(1)); // 222
  console.log(obj.instance.exports.switch(2)); // 333
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
wip
```

## 線形メモリーにアクセスしてみる

データの格納場所として線形メモリーを使うことができます。これは生のバイト列なようなもので、ロード、ストア命令によってモジュール内部から線形メモリにアクセスすることができます。

```
(module
  (memory (export "mem") 1)
  (func (export "add_each") (param $x i32) (param $offset i32) (param $length i32)
    (local $ptr i32)
    (local $end i32)
    ;; 初期化
    get_local $offset
    i32.const 4
    i32.mul
    tee_local $ptr
    get_local $length
    i32.const 4
    i32.mul
    i32.add
    set_local $end
    block $break
      loop $continue
        ;; ループのブレーク判定
        get_local $ptr
        get_local $end
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
