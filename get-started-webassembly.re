
={get-started} WebAssemblyをはじめよう

@<raw>{|latex|\clearpage}

この章では、wastで簡単な例を書いてみて、WebAssemblyにどんな機能があるかをおおまかに掴んでもらいます。

== 開発環境の構築

まずはWebAssemblyを開発、実行、デバッグするための環境を整えましょう。


=== WABT: The WebAssembly Binary Toolkit


WABT@<fn>{wabt}はwastからwasmへ変換したり、バイトコードのダンプやモジュールの詳細情報を可視化したりするコマンドラインツールを提供します。"wabbit"と発音します。

//footnote[wabt][https://github.com/WebAssembly/wabt]



//tsize[|latex||p{30mm}|p{110mm}|]
//table[tbl1][WABTの主なコマンドラインツール]{
ツール名	説明
-----------------
wast2wasm	wastからwasmへ変換する
wasm2wast	wasmからwastへ変換する
wasm-interp	wasmをデコードしてスタックベースのインタプリタで実行する
wast-desuger	標準化されたインタプリタがサポートするwast（S式、フラット形式、あるいは両方が混在する）をフラット形式に変換する
//}

==== インストール

githubのレポジトリからcloneしてmakeするだけです。

//cmd{
$ git clone --recursive https://github.com/WebAssembly/wabt
$ cd wabt
$ make
//}

=== Visual Studio Code + vscode-wast


wastを編集するためのvscode拡張（vscode-wast）を作りましたのでサンプルコードを書くときはVisual Studio Code + vscode-wastを前提として進めていきます。


==== Visual Studio Codeのインストール


Visual Studio Code公式サイト@<fn>{vsc}からVisual Studio Code本体をダウンロードしてインストールします。

//footnote[vsc][https://code.visualstudio.com/]

==== vscode-wastのインストール


コマンドパレットを開いて@<tt>{ext install wast}と入力してEnterを押してしばらく待つとvscode-wastがインストールされます。


==== wabtの設定


@<tt>{基本設定 > 設定}から設定を開いて以下の行を追加します。@<tt>{path/to/wabt}にはgit cloneしてきたwabtレポジトリのルートディレクトリを指定します。


//emlist{
"wabtPath": "path/to/wabt" 
//}

==== vscode-wastの機能


シンタックスハイライト、保存時のエラーチェックと各種コマンドが用意されています。


//image[vs-wast-syntax][シンタックスハイライト例][scale=0.5]{
//}



//image[vs-wast-errors][エラーチェック例]{
//}

//table[tbl2][コマンドパレットから実行できるコマンド一覧]{
コマンド	タイトル	説明
-----------------
@<tt>{wast.text}	Wast: Run tests	編集中のファイルをテストする
@<tt>{wast.dump}	Wast: Print a hexdump	編集中のファイルのhexdumpを表示する
@<tt>{wast.info}	Wast: Print a module infomation	編集中のファイルのモジュール情報を表示する
@<tt>{wast.build}	Wast: Build a wasm binary	編集中のファイルを同一ディレクトリにビルドする
//}

=== Webブラウザ


実行するだけならChrome、FirefoxのStable版で問題なく動作しますが、developer toolsでデバッグする場合はChrome Canary、Firefox Nightlyの使用を推奨します。どちらも読み込まれたwasmモジュールの可視化、ブレークポイントなどの機能を持っています。試してみた感じでは、Firefox Nightlyのほうが情報量が多いようです。



//image[wasm-debug-chrome][Chromeでのデバッグ例]{
//}




//image[wasm-debug-firefox][Firefoxでのデバッグ例]{
//}



== 関数を定義する


環境構築ができたら、wastを書いてみましょう。WebAssemblyのモジュールを作り、整数の足し算を行うadd関数を実装して、JavaScriptから使えるようにエクスポートしてみましょう。


//emlist{
(module
  (func
    (export "add")
    (param $x i32) (param $y i32) (result i32)
    get_local $x
    get_local $y
    i32.add)
)
//}


上のコードを少しずつ見ていきましょう。@<tt>{(module ...)}が1つのWebAssemblyモジュールを表します。@<tt>{(func ...)}で関数を定義していて、次の行の@<tt>{(export "add")}で@<tt>{add}というプロパティ名でアクセスできるように関数をエクスポートしています。@<tt>{(param $x i32) (pram $y i32) (result i32)}は関数の型を表しています。@<tt>{(param $x i32)}は32bit整数の引数を表し、@<tt>{$x}というラベルを使って関数内から参照することができます。@<tt>{(result i32)}は32bit整数を返すことを表します。



次に、@<tt>{get_local $x ...}からは関数のコード本体部分で、これはスタックマシンの命令列です。具体的には@<tt>{get_local $x}、@<tt>{get_local $y}でローカル変数@<tt>{$x}、@<tt>{$y}（引数はローカル変数に含まれます）から値を読み込んでスタックにpushし、@<tt>{i32.add}でスタックから2個の値をpopして足した結果をpushしています。



そして最後に、スタックに積まれている値が関数の戻り値となります。



wastは人間が読み書きするためのコードです。これをJSから読み込んで実行するためにwasm（バイナリ形式のコード）に変換します。上の内容をadd.wastとして保存して、wast2wasm、またはvscode-wastのビルド機能を使用してwasmに変換してみましょう。


//cmd{
$ wast2wasm add.wast -o add.wasm
//}


wasmファイルは@<tt>{fetch}（もちろんXHRでも可）でロードし、@<tt>{WebAssembly.instantiate}を用いて実行可能な形式にコンパイル、インスタンス化してWebブラウザで実行することができます。


//emlist[][js]{
fetch("add.wasm")
.then(res => res.arrayBuffer())                  // ArrayBufferとしてロード
.then(buffer => WebAssembly.instantiate(buffer)) // インスタンス化
.then(({module, instance}) => {
  // instance.exportsにエクスポートされたものが入ってきます
  console.log(instance.exports.add(1, 2));       // 3
});
//}

== インポートする


WebAssemblyには外部モジュールをインポートする仕組みがあります。@<tt>{console.log}をラップした関数をインポートとしてモジュール内部で使用してみましょう。


//emlist{
(module
  (import "foo" "print" (func $print (param i32)))
  (func (export "bar") (param $x i32)
    get_local $x
    call $print)
)
//}


@<tt>{(import "foo" "print" (func $print (param i32)))}がインポートの宣言です。@<tt>{"foo"}はインポートするモジュール名、@<tt>{"print"}がプロパティ名、@<tt>{(func $print (param i32))}が関数の型を表し、インポートされた関数は@<tt>{call $print}で呼ばれています。


//emlist[][js]{
const importObj = {
  foo: {
    print(x) { console.log(x) }
  }
};

fetch("import.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer, importObj))
.then(({module, instance}) => {
  instance.exports.bar(1); // importObj.foo.printが呼ばれます
});
//}


外部モジュールをインポートする場合、@<tt>{WebAssembly.instantiate}の第2引数にインポートするオブジェクトを指定します。オブジェクトの構造とwast内でのインポート宣言が対応していることに注目してください。


== if文的なことをする


いわゆるif文をWebAssemblyで実現するためにはifブロックを使用します。


//emlist{
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
//}


@<tt>{if}から@<tt>{end}までがifブロックを表します。スタックからpopした値が0でない場合のみifブロックに入ります。@<tt>{if}の直後の@<tt>{i32}はブロックを抜けるときにpushする値の型で、何もpushしない場合は型の部分を空にしてください。もしelseブロックが存在する場合popした値が0のときにそちらに飛びます。


//emlist[][js]{
fetch("if.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(({module, instance}) => {
  instance.exports.if(1);           // 100
  instance.exports.if(0);           // 10
  instance.exports.if_then_else(1); // 100
  instance.exports.if_then_else(0); // 10
});
//}

=={switch} switch文的なことをする


いわゆるswitch文をWebAssemblyで実現するためには@<tt>{block}と@<tt>{br_table}を組み合わせます。


//emlist{
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
//}


@<tt>{block}は対応する@<tt>{end}までのブロックを作ります。@<tt>{br_table}はネストしたブロックに対してスタックからpopした値をインデックスとして分岐する先を指定します。@<tt>{$a $b $c}が対象のブロックです。例えば、ローカル変数@<tt>{$i}が1ならば@<tt>{$b}のブロックを抜けて222が返ります。


//emlist[][js]{
fetch("switch.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(({module, instance}) => {
  console.log(instance.exports.switch(0)); // 111
  console.log(instance.exports.switch(1)); // 222
  console.log(instance.exports.switch(2)); // 333
});
//}

== 再帰呼び出しを行う


再帰関数、例として階乗を求めるfactorial関数を実装してみましょう。


//emlist{
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
//}

@<tt>{call $factorial}で自身を呼び出しています。これはJSと同じようなものですね。

== ループする



ループを実現するためにはloopブロックを使います。上で定義したfactorial関数をループで書き直してみましょう。


//emlist{
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
//}


@<tt>{loop}と、それに対応する@<tt>{end}までがloopブロックです。いわゆるwhile文のようなものを実装するときは、@<tt>{block}と合わせて使うことが多いです。@<tt>{loop}のすぐ後に出てくる@<tt>{br_if $exit}で終了判定をしていて、真なら外側のブロックを抜けます。@<tt>{loop}に対応する@<tt>{end}直前の@<tt>{br $cont}でloopブロックの先頭に飛びます。@<tt>{br}などの分岐命令はloopブロックなら先頭に、それ以外のブロックならブロックを抜けると覚えておいてください。


== グローバル変数を使う


モジュール内で共通してアクセスできるグローバル変数を定義することができます。実行する度に値をインクリメントするincrement関数を実装してみましょう。


//emlist{
(module
  (global $count (mut i32) (i32.const 0))

  (func (export "increment") (result i32)
    get_global $count
    i32.const 1
    i32.add
    set_global $count
    get_global)
)
//}


@<tt>{(global $count (mut i32) (i32.const 0))}がグローバル変数の定義です。@<tt>{(mut i32)}は変数のミュータビリティーと型を表し、@<tt>{(mut hoge)}と書くと値がミュータブルになります。もしもイミュータブルにしたい場合は単純に@<tt>{i32}と書いてください。@<tt>{(i32.const 0)}は変数のイニシャライザーで、定数や他のグローバル変数を参照することができます。グローバル変数にアクセスするときは@<tt>{get_global}、@<tt>{set_global}を使用します。


//emlist[][js]{
fetch("increment.wasm")
.then(res => res.arrayBuffer())
.then(buffer => WebAssembly.instantiate(buffer))
.then(({module, instance}) => {
  console.log(instance.exports.increment()); // 1
  console.log(instance.exports.increment()); // 2
  console.log(instance.exports.increment()); // 3
});
//}



== 線形メモリーにアクセスする


データの格納場所として線形メモリーを使うことができます。ロード、ストア命令によってモジュール内部から線形メモリーにアクセスすることができます。n個の32bit整数を合計した値を返すsum関数を実装してみましょう。


//emlist{
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
//}


@<tt>{(memory (export "mem") 1 2)}がメモリーの定義です。@<tt>{1 2}はメモリーの初期サイズと最大サイズを表します。メモリーを確保する最小単位は64KBで、この例の場合、初期サイズが64KB、最大サイズが128KBとなります。メモリーのサイズを拡張する場合は@<tt>{grow_memory}命令、現在のサイズを取得したい場合は@<tt>{current_memory}命令を使用します。次の行、@<tt>{(data (i32.const 0) "\01\00\00\00\02\00\00\00\03\00\00\00")}でオフセット0にバイト列@<tt>{"\01\00\00\00\02\00\00\00\03\00\00\00"}を挿入しています。メモリーからデータを読み込むために@<tt>{i32.load}を使用していて、具体的にはスタックからpopした値をオフセットして読み込んだデータをスタックにpushしています。その際、バイトオーダーはリトルエンディアンとして扱われます。


//emlist[][js]{
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
//}


メモリーをエクスポートする場合、@<tt>{WebAssembly.Memory}のインスタンスとしてエクスポートされ、データはArrayBufferとして@<tt>{buffer}プロパティからアクセスすることができます。


== 関数テーブルを使う


WebAssemblyでは指定した型の要素の参照を持つテーブルを定義できます。現状指定できる型は関数だけで、これは関数テーブルとして利用できます。@<hd>{switch}をテーブルを使って書き直してみましょう。


//emlist{
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
//}


@<tt>{(table (export "tbl") anyfunc 3 4)}がテーブルの定義です。@<tt>{anyfunc}が要素の型で、@<tt>{3 4}がテーブルの初期要素数と最大要素数を表します。次の行、@<tt>{(elem (i32.const 0) $f1 $f2 $f3)}でオフセット0に関数の参照@<tt>{$1 $2 $3}を挿入しています。テーブルから実際に関数を参照して呼び出すためには@<tt>{call_indirect}命令を使います。@<tt>{call_indirect}は参照する関数が、関数シグネチャー@<tt>{$return_i32}を持っていると期待して呼び出します。


//emlist[][js]{
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
//}


テーブルをエクスポートする場合、@<tt>{WebAssembly.Table}のインスタンスとしてエクスポートされ、関数の参照は@<tt>{get}メソッドから取得することができます。

