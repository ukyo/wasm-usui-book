
= WebAssemblyとは

@<raw>{|latex|\clearpage}

== WebAssemblyの概要

MDN@<fn>{wasm-abstract}から引用すると、WebAssemblyはモダンなWebブラウザで実行できる新しいタイプのコードで、新しい機能とパフォーマンスの大幅な向上をもたらします。基本的には、手書きではなく、C、C++、Rustなどの低レベルな言語からのコンパイル対象となるように設計されています。



WebAssemblyはWebプラットフォームに大きな影響を与えます。複数の言語で書かれたコードをWeb上でネイティブに近い速度で実行する方法を提供します。



さらに、WebAssemblyの作成方法を知らなくても恩恵を受けることができます。WebAssemblyモジュールをWeb（またはNode.js）アプリケーションにインポートして、WebAssemblyの関数をJavaScriptから使用することができます。JavaScriptフレームワークではWebAssemblyによってパフォーマンス向上や新機能を提供しながら、Web開発者は簡単に機能を利用することができます。

//footnote[wasm-abstract][https://developer.mozilla.org/en-US/docs/WebAssembly/Concepts]

== 現状できること（Minimum Viable Product）


WebAssemblyは必要最低限の機能（英語だとMinimum Viable Product、以下MVPと記述）だけを持ってリリースされました。大まかにはasm.jsと同等のことができて、具体的には以下のような機能を利用することができます。

 * WebAssemblyモジュールのソースコード（バイナリ）をJS API@<bib>{js-api}を用いてコンパイルし、ブラウザのJS環境上で動かすことができます。
 * 64bit整数やJSにはない低レベルの命令がサポートされています。
 * 変数、関数、線形メモリー、テーブル（参照を要素として持つ型付き配列で、現状は関数だけ）がサポートされています。
 * 上の機能に対してインポート、エクスポートするための仕組みが存在します。


== 将来的な機能


外部ツールの拡充、スレッド、SIMD、GC/DOM/Web APIの統合など、いつ実装されるかわかりませんが夢のある機能が控えています@<bib>{future-feature}。


== バイナリ表現とテキスト表現


WebAssemblyの内容はバイナリで表現されますが、人が読んだりデバッグしたり、または直接書くためのテキスト表現が存在します。本書では、そのテキスト表現を多用します。ただし、現状使われているテキスト表現の仕様は最終決定されたものではないので、本書のサンプルコードが使用できなくなる可能性があります。


以下、バイナリ表現をwasm、テキスト表現をwastと表記します。


== Webブラウザで実行されるまで


まずは、C、C++、Rust言語などからemscripten経由@<bib>{emcc-to-wasm}@<bib>{rust-to-wasm}で、もしくはwastから直接変換するかしてサーバー上にwasmを設置します。次にブラウザからwasmをダウンロードし、@<tt>{WebAssembly.compile}を用いてモジュールにコンパイルし、それを@<tt>{WebAssembly.instantiate}を用いて実行可能なインスタンスを生成してようやくJS環境上で動かすことができます。emscripten経由で変換された場合はそのあたりの面倒な部分をJSやhtmlに埋め込んでくれるので、ちょっと試すだけならこちらのほうが楽です。



//image[use-wasm][Webブラウザで実行されるまで][scale=0.5]{
//}



== ロゴ


WebAssemblyのロゴは、色々なものが提案されて、その中から投票によって決定されました@<fn>{logo-vote}。ロゴのライセンスはCC0です。

//footnote[logo-vote][https://github.com/WebAssembly/design/issues/980]

//image[web-assembly-logo-black][WebAssemblyロゴ][scale=0.5]{
//}

@<raw>{|latex|\clearpage}
　