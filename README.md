# wasm-usui-book
webassemblyの薄い本

WebAssenbly is available in Firefox 52 now. Go to https://ukyo.github.io/wasm-usui-book/examples/ and run below codes.

```js
fetch("add.wasm")
.then(res => res.arrayBuffer()) // ArrayBufferとしてロード
.then(buffer => WebAssembly.instantiate(buffer)) // インスタンス化
.then(obj => {
  // instanceがインスタンスでexportsにエクスポートされたものが入ってきます
  console.log(obj.instance.exports.add(1, 2)); // 3
});
```
