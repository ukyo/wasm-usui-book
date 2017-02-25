# Get Started WebAssembly

まずは最小のvalidなwasmモジュールを作ってみます。最小のモジュールは空のモジュールです。

```
(module)
```

empty.wasとして保存してwasmにコンパイルします。

```
wast2wasm empty.was -o empty.wasm
```

ブラウザーで実行します。

```js
fetch("empty.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(obj => {
  console.log(obj.instance.exports); // 空
});
```

