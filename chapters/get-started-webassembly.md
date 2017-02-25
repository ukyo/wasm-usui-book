# Get Started WebAssembly

まずは最小のwasmモジュールを作ってみます。最小のモジュールは空のモジュールです。

```
(module)
```

empty.wasとして保存してwasm

```
wast2wasm empty.was -o empty.wasm
```

```js
fetch("empty.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(({module, instance}) => {
  console.log(instance.exports);
});
```
