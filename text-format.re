
= WebAssembly テキスト表現

@<raw>{|latex|\clearpage}

この章ではWebAssemblyのテキスト表現@<bib>{wast-spec}について解説します。基本的にはバイナリ表現と対応していますが、省略記法などの独特の記法もあるので都度解説します。これは、執筆時点では公式のテキスト表現として使用されていますが、あくまで暫定的なもので、変更される可能性があることに注意してください。

== 型

値の型は@<tt>{i32}、@<tt>{i64}、@<tt>{f32}、@<tt>{f64}、@<tt>{anyfunc} の4種類です。テーブルの要素の型は@<tt>{anyfunc}の1種類です。



== コメント

いわゆるコメントです。コメントはソース上無視されます。



//emlist{
;; 一行コメント

(;
ブロックコメント
;)
//}

== ラベル


バイナリ表現では他のセクションのエントリーやローカル変数などを参照するときに直接インデックスを指定していましたが、テキスト表現ではラベルを指定（例: @<tt>{$foo}、@<tt>{$1-2-3}）することで、インデックスの代わりにラベルで参照することができます。


== module


モジュール。wast2wasmでコンパイルする場合、1ファイルに複数のモジュールを定義してもエラーは起きませんが、最初に定義したモジュールがコンパイル結果となります。


//emlist{
(module
  ;; 各セクションのエントリーが入ります
)
//}

== セクション

=== type


関数シグネチャーの宣言をします。


//emlist{
(module
  (type (func))         ;; 引数が0個で返り値がない関数
  (type $label (func))  ;; ラベル付き
  ;; i32型の引数を一つ受け取り、i32型の値を返す
  (type $type1 (func (param i32) (result i32)))
  ;; i32型の引数を一つ、i64型の引数を1つ受け取り、f64型の値を返す 
  (type $type2 (func (param i32 i64) (result f64)))
)
//}

=== func


関数宣言とコード本体の定義をセットで行います。


//emlist{
(module
  (func)                                          ;; 何もしない関数
  (func $nop)                                     ;; ラベル付き
  (func (param i32))                              ;; i32型の引数を1つ受け取る
  (func (param i32 i64))                          ;; i32型、i64型の引数を受け取る
  (func (param $x i32))                           ;; 引数にラベル付けてみる
  (func (param $x i32) (param $y i64))            ;; 複数の場合はparamを分けて書く
  (func
    (param i32)                                   ;; i32型の引数を1つ受け取り
    (result i32)                                  ;; i32型の値を返す
    get_local 0)
  (func (param i32) (result i32)
    (local i32)                                   ;; ローカル変数の宣言
    (local $a i32)                                ;; ラベル付き
    i32.const 1
    set_local 1
    i32.const 2
    tee_local $a
    get_local 0
    i32.add)
  (func (type $type1) (param $x i32) (result i32) ;; typeを参照する
    get_local $x)
  (func (type $type1)                             ;; 型の記述は省略できる
    get_local 0)
)
//}

=== start


インスタンス化が完了したときに呼ばれるfuncを指定します。


//emlist{
(module
  (func $foo)
  (start $foo) ;; function indexかラベルで指定する
)
//}

=== table, elem


tableを定義します。elemを用いて対象のオフセットに要素を挿入します。tableは各モジュールに対して1個以下である必要があります。


//emlist{
(module
  (table 10 anyfunc)              ;; 型がanyfuncで10個の領域を持つtableを作成
)

(module
  (table $tbl 10 anyfunc)         ;; ラベル付き
)

(module
  (table 10 20 anyfunc)           ;; 初期で10個、最大で20個まで拡張できるtableを作成
                                  ;; wasmモジュール内での拡張できない
)

(module
  (table 10 anyfunc)
  (elem (i32.const 0) $a)         ;; initializerで指定したoffset(0)に要素を流し込む
  (elem (i32.const 1) $a $b)      ;; offset=1から順番に流し込む
  (func $a)
  (func $b)
)

(module
  (table anyfunc (elem $a $b $c)) ;; 略記法。要素数が3で、それぞれ$a $b $cの
                                  ;; function indexを持つ要素が入る
  (func $a)
  (func $b)
  (func $c)
)

;; call_indirectの例
(module
  (type $foo (func (param i32 i32) (result i32)))
  (type $bar (func (param i32) (result i32)))
  (table anyfunc (elem $add $sub $mul $add2 $sub2))

  (func $add (type $foo) get_local 0 get_local 1 i32.add)
  (func $sub (type $foo) get_local 0 get_local 1 i32.sub)
  (func $mul (type $foo) get_local 0 get_local 1 i32.mul)
  (func $add2 (type $bar) get_local 0 i32.const 2 i32.add)
  (func $sub2 (type $bar) get_local 0 i32.const 2 i32.sub)

  (func $call_foo (param $i i32) (result i32)
    i32.const 10 i32.const 5  ;; 引数2個で
    get_local $i              ;; tableの$i番目の要素から参照されている関数を
    call_indirect $foo)       ;; typeが$fooであると期待して呼ぶ

  (func $call_bar (param $i i32) (result i32)
    i32.const 4               ;; 引数1個で
    get_local $i              ;; tableの$i番目の要素から参照されている関数を
    call_indirect $bar)       ;; typeが$barであると期待して呼ぶ
)
//}

=== memory, data


memoryを定義します。memoryはモジュールに対して1個以下である必要があります。dataで対象のオフセットにバイト列を挿入します。


//emlist{
(module (memory 10))             ;; 10ページ分(10 * 64KB)のメモリー確保
;; 初期値で10ページ確保。必要になったら最大20ページまで領域を拡張できる
(module (memory 10 20))                  
(module
  (memory $memory 10)            ;; ラベル付き
  ;; イニシャライザーで指定したオフセット(0byte目)から
  ;; UTF8でエンコーディングされたバイト列としてデータを流し込む
  (data (i32.const 0) "abc\01\ffあいう") 
  (data (i32.const 128) "iiiii") ;; offset(128バイト目)から流し込む
) 
(module (memory (data "abc"))    ;; 略記法、必要な分だけ確保してバイト列を先頭から流し込む

//}

=== global


グローバル変数を定義します。


//emlist{
(module
  ;; initializerの返却値を初期値としたイミュータブルなグローバル変数を定義
  (global i32 (i32.const 1))               
  (global $foo i64 (i64.const 100))        ;; ラベル付き
  (global $bar (mut f32) (f32.const 1.11)) ;; ミュータブルなグローバル変数
)
//}

=== import


func、global、memoryまたはtableをインポートします。インポートされたmemory、tableにもモジュール内に1個までという制限が適用されます。


//emlist{
;; importメインの記法
(module
  (import "mod1" "prop1" (func $f (param i32)))
  (import "mod1" "prop2" (global $x i32))
  (import "mod2" "prop1" (memory $mem 2))
  (import "mod2" "prop2" (table $tbl 10 anyfunc))
)

;; func, global, memoryまたはtableメインの記法
(module
  (func $f (import "mod1" "prop1") (param i32))
  (global $x (import "mod1" "prop2") i32)
  (memory $mem (import "mod2" "prop1") 2)
  (table $tbl (import "mod2" "prop2") 10 anyfunc)
)
//}


以下のように@<tt>{WebAssembly.instaniate}からインポートするオブジェクトを設定します。


//emlist[][js]{
const importObject = {
  mod1 : {
    prop1: x => { /* ... */ },
    prop2: 100
  },
  mod2: {
    prop1: new WebAssembly.Memory({ initial: 2 }),
    prop2: new WebAssembly.Table({ initial: 10 })
  }
};
WebAssembly.instaniate(bufferSource, importObject);
//}

=== export


func、global、memoryまたはtableをエクスポートします。


//emlist{
;; 定義とexportを分けて書く記法
(module
  (func $f (param i32))
  (global $x i32 (i32.const 100))
  (memory $mem 2)
  (table $tbl anyfunc (elem $f))

  (export "f" (func $f))
  (export "x" (global $x))
  (export "mem" (memory $mem)) ;; 現状memoryは最大1個なので基本的に(memory 0)でOK
  (export "tbl" (table $tbl))  ;; 上と同じ理由で(table 0)でOK
)

;; 定義に埋め込む記法
(module
  (func $f (export "f") (param i32))
  (global $x (export "x") i32 (i32.const 100))
  (memory $mem (export "mem") 2)
  (table $tbl (export "tbl") anyfunc (elem $f))
)
//}


エクスポートされたfunc、global、memory、tableはJS実行環境上でwasmモジュールのインスタンスから参照できます。


//emlist[][js]{
WebAssembly.instaniate(bufferSource).then(obj => {
  obj.instance.exports.f;   // exported function
  obj.instance.exports.x;   // 100 イミュータブル
  obj.instance.exports.mem; // WebAssembly.Memoryオブジェクト
  obj.instance.exports.tbl; // WebAssembly.Tableオブジェクト
});
//}

== コード本体


funcのコード本体部分について。WebAssemblyのバイナリ表現ではスタックマシンの命令列として記述されます。テキスト表現においては、バイトコードをそのままテキストに置き換えたようなフラット形式か、S式、または両方を混ぜて記述することができます。


=== 基本的な命令


フラット形式。スペース区切りで@<tt>{opcode immediate*}を1セットとして並べるだけです。


//emlist{
i32.const 1
i32.const 2
i32.add
i32.const 3
i32.sub
//}


S式。普通にS式で構文木を作るだけです。


//emlist{
(i32.sub
  (i32.add (i32.const 1) (i32.const 2))
  (i32.const 3))
//}

=== フロー系の命令

==== @<tt>{block}


フラット形式。対となる@<tt>{end}までをブロックとして扱います。


//emlist{
block $foo ;; ブロックにラベルを付けられる。可読性のために付けることを推奨。
;; ...
end

block $bar f64 ;; value typeを指定することでブロック終了時に値をpushします
  f64.const 1.1
end
//}


S式の場合は@<tt>{end}は書きません。


//emlist{
(block $foo
  ;; ...
)

(block $bar f64
  f64.const 1.1
)
//}

==== @<tt>{loop}


@<tt>{block}と同じなので省略します。


==== @<tt>{if}、@<tt>{else}


フラット形式。


//emlist{
i32.const 1
if            ;; スタックからオペランドをpopして0でなければブロックに入る
;; ...
end

i32.const 1
if i32        ;; 値をpushするifブロック
  i32.const 100
end

i32.const 0
if
;; ...
else          ;; オペランドが0ならこちらに飛ぶ
;; ...
end
//}


S式。真の場合は@<tt>{then}、偽の場合は@<tt>{else}に飛びます。


//emlist{
(if (i32.const 1)
  (then
    ;; ...
  ))

(if i32 (i32.const 1)
  (then (i32.const 100)))

(if (i32.const 0)
  (then
    ;; ...
  )
  (else
    ;; ...
  ))
//}

==== @<tt>{br}、@<tt>{br_if}、@<tt>{br_table}


対象のブロックに対する分岐命令。block、ifブロックならブロックを抜けます。loopブロックならブロック先頭に飛びます。



フラット形式。


//emlist{
block $foo
  block $bar
    br $foo                 ;; 対象ブロックに対して分岐(この場合$fooブロックを抜ける)
  end
end

block $foo
  block $bar
    i32.const 1
    br_if $foo              ;; popしたオペランドが非ゼロなら分岐
  end
end

block $foo
  block $bar
    i32.const 0
    ;; popしたオペランドをiとして即値オペランド列の
    ;; i番目に指定されているブロックに対して分岐
    ;; 一番最後の即値オペランドはデフォルト値として扱われる
    br_table $foo $bar $foo
  end
end
//}


S式。


//emlist{
(block $foo
  (block $bar
    br $foo
  )
)

(block $foo
  (block $bar
    (br_if $foo (i32.const 1))
  )
)

(block $foo
  (block $bar
    (br_table $foo $bar $foo (i32.const 0))
  )
)
//}

=== load, store系オペレータ


load、store系命令でオフセットとアライメントを指定する場合は以下のように記述します。



フラット形式。


//emlist{
i32.const 1
i32.load offset=0         ;; オフセット指定

i32.const 1
i32.load align=4          ;; アライメント指定(2の乗数であること)

i32.const 1
i32.load offset=0 align=4 ;; 両方指定する場合はオフセットを先に書くこと
//}


S式。


//emlist{
(i32.load offset=0 (i32.const 1))
(i32.load align=4 (i32.const 1))
(i32.load offset=0 align=4 (i32.const 1))
//}

@<raw>{|latex|\clearpage}

== テスト


モジュール自身やエクスポートされた関数をテストすることができます。何種類かあるアサーションのうち@<tt>{assert_return}を紹介します。

//emlist{
(module
  (global (export "val") i32 (i32.const 100))
  (func (export "add") (param $x i32) (param $y i32) (result i32)
    get_local $x
    get_local $y
    i32.add)
)
(assert_return (get "val") (i32.const 100))
(assert_return (invoke "add" (i32.const 10) (i32.const 20)) (i32.const 30))
//}

@<tt>{(assert_return <action> <expected>)}の形式でアサーションを記述します。@<tt>{get}でグローバル変数、@<tt>{invoke}で関数のテストをすることができます。実際にテストするにはrun-interp.pyを使用するのが一番手軽です。

//cmd{
$ path/to/wabt/test/run-interp.py --spec foo.wast
2/2 tests passed.
//}