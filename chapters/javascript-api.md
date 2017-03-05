# JavaScript API

`WebAssembly`オブジェクトにメソッドとコンストラクタが定義されています。

## メソッド

### WebAssembly.compile

```
Promise<WebAssembly.Module> WebAssembly.compile(bufferSource)
```

wasmモジュールのソース（typed arrayかArrayBuffer）を`WebAssembly.Module`オブジェクトにコンパイルします。


### WebAssembly.instantiate

`WebAssembly.instantiate`は2種類の呼び出し方があります。

```
Promise<ResultObject> WebAssembly.instantiate(bufferSource, importObject);
```

wasmモジュールのソース（typed arrayかArrayBuffer）をインスタンス化します。`ResultObject`は以下の2個のフィールドを持ちます。

* module: `WebAssembly.Module`オブジェクト
* instance: `WebAssembly.Instance`オブジェクト

```
Promise<WebAssembly.Instance> WebAssembly.instantiate(module, importObject)
```

コンパイルされたwasmモジュールをインスタンス化します。`WebAssembly.Instance`オブジェクトを返します。

### WebAssembly.validate

```
WebAssembly.validate(bufferSource);
```

typed arrayまたはArrayBufferがwasmモジュールのソースとして正しいかどうかチェックします。


## コンストラクタ

### WebAssembly.Module

```
var myModule = new WebAssembly.Module(bufferSource);
```

`WebAssembly.compile`と同じようなものですが、こちらは同期的に実行されます。


### WebAssembly.Instance

```
var myInstance = new WebAssembly.Instance(module, importObject);
```

同期的な`WebAssembly.instantiate`。

### WebAssembly.Memory

`WebAssembly.Memory`オブジェクトを生成します。

```
var myMemory = new WebAssembly.Memory(memoryDescriptor);
```

### WebAssembly.Table

`WebAssembly.Table`オブジェクトを生成します。

```
var myTable = new WebAssembly.Table(tableDescriptor);
```
