
={wasm-encoding} WebAssembly バイナリ表現

@<raw>{|latex|\clearpage}

この章ではwasmモジュールのバイナリ表現について解説します。まずは内部で使用されるデータ型、次にモジュールの構造について見ていきます。この章の内容は公式の資料@<bib>{wasm-bin}を翻訳、補足したものです。


=={data-type} データ型

=== 数値（Numbers）

==== @<tt>{uintN}


符号なしの @<b>{N} bitの整数値、リトルエンディアンで表現されます。@<tt>{uint8}、 @<tt>{uint16}、 @<tt>{uint32} の3種類が使用されています。


==== @<tt>{varuintN}


LEB128@<bib>{LEB128}で表現される @<b>{N} bitの符号なし整数です。


//note{
現在 @<tt>{varuint1}, @<tt>{varuint7}, @<tt>{varuint32}のみ使用されてます。前者2つは将来的な拡張機能と互換性のために使用されています。
//}

==== @<tt>{varintN}


符号付きLEB128で表現される @<b>{N} bitの整数です。



//note{
  現在 @<tt>{varint7}, @<tt>{varint32}, @<tt>{varint64}が使用されています。
//}

==={opcode} オペコード（Instruction Opcodes）


MVPでは、オペコードの個数はは256以下であるため1バイトで表現されます。将来的にSIMDやアトミック操作の命令を追加すると256を超えるため、拡張スキーマが必要になります。マルチバイトオペコード用に1バイトのプレフィックス値を策定中です。


=== 言語型（Language Types）


全ての型（型コンストラクタを表す）は先頭の負の@<tt>{varint7}値によって識別されます。

//tsize[|latex||p{36mm}|p{30mm}|p{70mm}|]
//table[tbl1][]{
オペコード(@<tt>{varint7}値)	オペコード(バイト値)	Type constructor
-----------------
@<tt>{-0x01}	@<tt>{0x7f}	@<tt>{i32}
@<tt>{-0x02}	@<tt>{0x7e}	@<tt>{i64}
@<tt>{-0x03}	@<tt>{0x7d}	@<tt>{f32}
@<tt>{-0x04}	@<tt>{0x7c}	@<tt>{f64}
@<tt>{-0x10}	@<tt>{0x70}	@<tt>{anyfunc}
@<tt>{-0x20}	@<tt>{0x60}	@<tt>{func}
@<tt>{-0x40}	@<tt>{0x40}	空の@<tt>{block_type}を表現する擬似的な型
//}


この中でいくつかは追加のフィールドが続くものがあります（後述）。



//note{
  将来的な拡張性のために隙間の数値は予約されています。符号付きの数値（つまりここで負の値）を使っているは1バイトでtype sectionへのインデックス（後述）を設定できるようにするためです。型システムの将来的な拡張に関連しています。
//}

==== @<tt>{value_type}


値型を表す@<tt>{varint7}値。@<tt>{i32}、@<tt>{i64}、@<tt>{f32}、@<tt>{f64}の中の一つです。


==== @<tt>{block_type}


ブロック（後述）が返す型を表す@<tt>{varint7}値。ブロックが値を返す場合は@<tt>{value_type}、何も返さない場合は@<tt>{-0x40}が設定されます。


==== @<tt>{elem_type}


テーブル（後述）中の要素の型を表す@<tt>{varint7}値。MVPでは@<tt>{anyfunc}のみ有効です。



//note{
  将来的に他の要素も有効になる可能性があります。
//}

==== @<tt>{func_type}


関数シグネチャーを表します。この型コンストラクタには以下の記述が追加されます。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl2][]{
フィールド	型	説明
-----------------
form	@<tt>{varint7}	上の表で定義した@<tt>{func}型コンストラクタの値
param_count	varuint32	関数のパラメータ数
param_types	@<tt>{value_type*}	関数のパラメータの型(パラメータ数だけ用意)
return_count	@<tt>{varuint1}	関数の返り値の数
return_type	@<tt>{value_type?}	関数の返り値の型(return_countが1なら)
//}


//note{
  将来的に@<tt>{return_count}と@<tt>{return_type}は複数の値を許容するために一般化されるかもしれません。
//}

=== 他の型（Other Types）

==== @<tt>{global_type}


グローバル変数の情報を表します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl3][]{
フィールド	型	説明
-----------------
content_type	@<tt>{value_type}	値の型
mutability	@<tt>{varuint1}	@<tt>{0}ならイミュータブル。@<tt>{1}ならミュータブル
//}

==== @<tt>{table_type}


テーブルの情報を表します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl4][]{
フィールド	型	説明
-----------------
element_type	@<tt>{elem_type}	要素の型
limits	@<tt>{resizable_limits}	後述
//}

==== @<tt>{memory_type}


メモリーの情報を表します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl5][]{
フィールド	型	説明
-----------------
limits	@<tt>{resizable_limits}	後述
//}

==== @<tt>{external_kind}


インポートまたはモジュール内で定義された定義の種類を表す1バイトの符号なし整数値。

//tsize[|latex||p{25mm}|p{115mm}|]
//table[tbl6][]{
Value	Description
-----------------
@<tt>{0}	インポートまたはモジュール内で定義された @<tt>{Function}
@<tt>{1}	インポートまたはモジュール内で定義された @<tt>{Table}
@<tt>{2}	インポートまたはモジュール内で定義された @<tt>{Memory}
@<tt>{3}	インポートまたはモジュール内で定義された @<tt>{Global}
//}

==== @<tt>{resizable_limits}


テーブル、メモリーの初期、最大サイズを表します。テーブルの場合は要素数、メモリーの場合は64KBを1単位とします。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl7][]{
フィールド	型	説明
-----------------
flags	@<tt>{varuint1}	maximumフィールドがあるかどうか
initial	@<tt>{varuint32}	初期の長さ
maximum	@<tt>{varuint32?}	最大値（@<tt>{flags}が@<tt>{1}ならこのフィールドがある）
//}


//note{
  @<tt>{flags} フィールドは@<tt>{varuint32}に変わる可能性があります。例えばスレッド間で共有するためのフラグが含まれたりする。
//}

==== @<tt>{init_expr}


イニシャライザー。code section（後述）で用いられている式と@<tt>{end}オペコードによって構成されます。



//note{
  イニシャライザーに含まれる@<tt>{get_global}はインポートされたイミュータブルなグローバル変数のみ参照でき、全ての@<tt>{init_expr}はimport sectionの後でのみ使用できます。
//}

@<raw>{|latex|\clearpage}

== モジュール構造（Module structure）

WebAssemblyモジュールの高レベルな構造（セクションの集合）と各セクションについて解説します。wast2wasmでモジュールの詳細情報を確認できるので、合わせて見てもらえると理解の助けになると思われます。

//cmd{
$ wast2wasm -v add.wast
//}

//image[wasm-info][wast2wasm -v の実行例][scale=0.7]{
//}


=== 高レベルな構造（High-level structure）


//image[wasm-binary][wasmモジュール概要図]{
//}




モジュールは以下の2つのフィールドから開始します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl8][]{
フィールド	型	説明
-----------------
magic number	@<tt>{uint32}	マジックナンバー @<tt>{0x6d736100} （@<tt>{"\0asm"}）
version	@<tt>{uint32}	バージョンナンバー（@<tt>{0x1}）
//}


モジュールプリアンブルのあとに一連のセクションが続きます。各セクションは既知のセクションまたはカスタムセクションを表す1バイトのセクションコードによって識別されます。そして、セクションの長さ、ペイロードが次に続きます。既知のセクションは非ゼロなidを持ちます。カスタムセクションのidは@<tt>{0}で、その後にペイロードの一部として識別文字列が続きます。カスタムセクションはWebAssemblyの実装では無視されるので内部でのバリデーションエラーがあってもモジュールが無効になるわけではありません。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl9][]{
フィールド	型	説明
-----------------
id	@<tt>{varuint7}	セクションコード
payload_len	@<tt>{varuint32}	セクションのバイト長
name_len	@<tt>{varuint32?}	セクション名のバイト長。@<tt>{id == 0}のときだけ存在します
name	@<tt>{bytes?}	セクション名の本体。@<tt>{id == 0}のときだけ存在します
payload_data	セクションの中身。長さは @<tt>{payload_len - sizeof(name) - sizeof(name_len)}	.
//}


既知のセクションはオプショナルで最大で1個だけです。カスタムセクションは全て同じidを持ちユニークでない名前をつけることができます。以下の表で定義される既知のセクションは順不同で現れない場合もあります。各セクションの@<tt>{payload_data}にエンコードされた内容が入ります。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl10][]{
セクション名	コード	説明
-----------------
Type	@<tt>{1}	関数シグネチャー宣言
Import	@<tt>{2}	インポート宣言
Function	@<tt>{3}	関数宣言
Table	@<tt>{4}	間接的な関数テーブルと他のテーブル
Memory	@<tt>{5}	メモリー属性
Global	@<tt>{6}	グローバル宣言
Export	@<tt>{7}	エクスポート
Start	@<tt>{8}	開始関数宣言
Element	@<tt>{9}	要素セクション
Code	@<tt>{10}	関数本体（code）
Data	@<tt>{11}	データセグメント
//}


最後のセクションの最終バイトはモジュールの最終バイトと一致する必要があります。最小のモジュールは8バイトです（@<tt>{magic number}, @<tt>{version}のあとに何もない状態）。


=== Type section


Type sectionでは、モジュールで使用されている全ての関数シグネチャーを宣言します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl11][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	エントリー数
entries	@<tt>{func_type*}	関数シグネチャー列
//}


//note{
  将来的に、他の型を持つエントリーもここに入る可能性があります。@<tt>{func_type}の@<tt>{form}フィールドで識別できます。
//}

=== Import section


Import sectionではモジュールで使用されている全てのインポートを宣言します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl12][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	エントリー数
entries	@<tt>{import_entry*}	import entry（後述）列
//}

各@<tt>{import_entry}は以下のような構造を持ちます。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl13][]{
フィールド	型	説明
-----------------
module_len	@<tt>{varuint32}	モジュール文字列の長さ
module_str	@<tt>{bytes}	@<tt>{module_len}バイトのモジュールの文字列
フィールド_len	@<tt>{varuint32}	フィールド名の長さ
フィールド_str	@<tt>{bytes}	@<tt>{フィールド_len}バイトのフィールド名
kind	@<tt>{external_kind}	インポートされた定義の種類
//}


@<tt>{kind}の種類によって以下の項目が続きます。@<tt>{kind}が@<tt>{Function}の場合は

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl14][]{
フィールド	型	説明
-----------------
type	@<tt>{varuint32}	type sectionにある関数シグネチャーのインデックス
//}


@<tt>{kind}が@<tt>{Table}の場合は

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl15][]{
フィールド	型	説明
-----------------
type	@<tt>{table_type}	インポートされたテーブルの情報
//}


@<tt>{kind}が@<tt>{Memory}の場合は

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl16][]{
フィールド	型	説明
-----------------
type	@<tt>{memory_type}	インポートされたメモリーの情報
//}


@<tt>{kind}が@<tt>{Global}の場合は

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl17][]{
フィールド	型	説明
-----------------
type	@<tt>{global_type}	インポートされたグローバル変数の情報
//}


//note{
  MVPではイミュータブルなグローバル変数のみインポートできます。
//}

=== Function section


Function sectionではモジュール内のすべての関数シグネチャーを宣言します（関数の定義はcode sectionに置かれます）。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl18][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	エントリー数
types	@<tt>{varuint32*}	type sectionに置いてある対象の@<tt>{func_type}のインデックス列
//}

=== Table section


Table sectionではモジュール内で使用するテーブル（参照を要素として持つ型付き配列）を定義します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl19][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	モジュールに定義されているテーブルの数
entries	@<tt>{table_type*}	@<tt>{table_type}エントリー列
//}


//note{
  MVPでは、テーブルの数は1以下でなければいけません。@<tt>{table_type}で使用できる型は@<tt>{anyfunc}だけで、関数テーブルとして機能します。
//}

=== Memory section


Memory sectionではモジュール内で使用する線形メモリーを定義します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl20][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	モジュールで定義されているメモリーの数
entries	@<tt>{memory_type*}	@<tt>{memory_type} エントリー列
//}


//note{
MVPでは、メモリーの数は1以下でなければいけません。
//}

=== Global section


Global sectionではモジュール内で使用するグローバル変数を定義します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl21][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	グローバル変数エントリーの数
globals	@<tt>{global_variable*}	後述のグローバル変数列
//}

各@<tt>{global_variable}は以下のような構造を持ちます。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl22][]{
フィールド	型	説明
-----------------
type	@<tt>{global_type}	変数の型
init	@<tt>{init_expr}	グローバル変数のイニシャライザー
//}

@<tt>{global_type}によって型とミュータビリティー、@<tt>{init_expr}によって初期値が与えられます。

//note{
  MVPではイミュータブルなグローバル変数だけエクスポートできます。
//}

=== Export section


Export sectionではモジュール外にエクスポートするエントリーを宣言します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl23][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	エントリー数
entries	@<tt>{export_entry*}	後述のエクスポートエントリー列
//}

@<tt>{export_entry}は以下のような構造を持ちます。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl24][]{
フィールド	型	説明
-----------------
フィールド_len	@<tt>{varuint32}	フィールド名の長さ
フィールド_str	@<tt>{bytes}	@<tt>{フィールド_len}バイトのフィールド名
kind	@<tt>{external_kind}	エクスポートされた定義の種類
index	@<tt>{varuint32}	対応するindex space（後述）へのインデックス
//}


例えば、@<tt>{kind}が@<tt>{Function}のときはindexはfunction section中でのインデックス(function index)になります。MVPではメモリ、テーブルは0だけです。


=== Start section


Start sectionではモジュールのインスタンス化が完了したときに呼ばれる関数を宣言します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl25][]{
フィールド	型	説明
-----------------
index	@<tt>{varuint32}	開始する関数のfunction index
//}

=== Element section


Element sectionではモジュールをインスタンス化するときに初期値としてテーブルに設定する要素を宣言します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl26][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	エレメントセグメントの数
entries	@<tt>{elem_segment*}	後述のエレメントセグメント列
//}


@<tt>{elem_segment}は以下ような構造を持ちます。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl27][]{
フィールド	型	説明
-----------------
index	@<tt>{varuint32}	table index（MVPでは0）
offset	@<tt>{init_expr}	要素の場所のオフセットを返すイニシャライザー（@<tt>{i32}値）
num_elem	@<tt>{varuint32}	要素数
elems	@<tt>{varuint32*}	function index列
//}

=== Code section


Code sectionにはモジュール内の全ての関数の本体が含まれています。function sectionで宣言された関数の数と関数本体の定義は必ず一致しなければいけません。@<tt>{i}番目の関数宣言は@<tt>{i}番目の関数本体に対応しています。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl28][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	@<tt>{function_body}の数
bodies	@<tt>{function_body*}	関数本体の列（Function bodiesで説明）
//}

=== Data section


Data sectionではモジュールをインスタンス化するときにロードされる初期化データを宣言します。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl29][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	データセグメントの数
entries	@<tt>{data_segment*}	後述のデータセグメント列
//}


@<tt>{data_segment}は以下のような構造を持ちます。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl30][]{
フィールド	型	説明
-----------------
index	@<tt>{varuint32}	線形メモリーのインデックス（MVPでは0）
offset	@<tt>{init_expr}	データの場所のオフセットを返すイニシャライザー（@<tt>{i32}値）
size	@<tt>{varuint32}	@<tt>{data}のバイト数
data	@<tt>{bytes}	データ本体
//}

=== Name section


Name sectionはカスタムセクションの1つです。@<tt>{name}フィールドに@<tt>{"name"}が設定されています。全てのカスタムセクション同様、このセクションでのバリデーションエラーは全体のモジュールには影響しません。
Name sectionは（あれば）データセクションのあとに1回だけ現れます。wasmモジュールやその他、開発環境で表示する場合、Name sectionで定義される関数名やローカル変数名はWebAssemblyのテキスト表現などで使用されることが期待されます。影響しないのでここでは構造についての説明は省略します。


== 関数本体（Function Bodies）


関数本体は一連のローカル変数宣言の後にバイトコード命令が続きます。これは構造化されたスタックマシンの命令列とみなすことができます。命令はオペコードと0個以上の即値オペランド（immediates）にエンコードされます。各関数本体は@<tt>{end}オペコードで終わらなければなりません。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl31][]{
フィールド	型	説明
-----------------
body_size	@<tt>{varuint32}	関数本体のバイト数
local_count	@<tt>{varuint32}	ローカル変数の数
locals	@<tt>{local_entry}	ローカル変数列
code	@<tt>{byte*}	関数のバイトコード
end	`byte	@<tt>{0x0b}、関数本体の終了
//}

=== Local entry


各ローカルエントリーは与えられた型のローカル変数リストを宣言します。同じ型のエントリーを複数持つことができます。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl32][]{
フィールド	型	説明
-----------------
count	@<tt>{varuint32}	ローカル変数の数
type	@<tt>{value_type}	変数の型
//}

== オペコード一覧


wasmで定義されているオペコードを紹介します。オペコードがオペランドを受け取る場合はN番目のオペランドを@<tt>{opN}とします。


=== Control flow operators

//tsize[|latex||p{19mm}|p{15mm}|p{37mm}|p{61mm}|]
//table[tbl33][]{
名前	オペコード	即値	説明
-----------------
@<tt>{unreachable}	@<tt>{0x00}	.	即例外を発生
@<tt>{nop}	@<tt>{0x01}	.	なにもしないオペコード
@<tt>{block}	@<tt>{0x02}	sig:@<tt>{block_type}	式のシーケンスを開始して、0個か1個の値をpush
@<tt>{loop}	@<tt>{0x03}	sig:@<tt>{block_type}	ループできるブロックを作る
@<tt>{if}	@<tt>{0x04}	sig:@<tt>{block_type}	if式。@<tt>{op1 != 0}のときブロックに入る
@<tt>{else}	@<tt>{0x05}	.	ifに対応するelse式
@<tt>{end}	@<tt>{0x0b}	.	@<tt>{block}、@<tt>{if}、@<tt>{loop}の終了
@<tt>{br}	@<tt>{0x0c}	relative_depth:@<tt>{varuint32}	対象の親ブロックに対して分岐命令を行う（以下補足）
@<tt>{br_if}	@<tt>{0x0d}	relative_depth:@<tt>{varuint32}	@<tt>{br}の条件分岐あり版(以下補足)
@<tt>{br_table}	@<tt>{0x0e}	後述	後述
@<tt>{return}	@<tt>{0x0f}	.	関数から0個か1個の値を返す
//}

==== 分岐命令について


@<tt>{br}, @<tt>{br_if}, @<tt>{br_table} は分岐命令を行う対象のブロックによって挙動が変わります。@<tt>{block}, @<tt>{if} ブロックでは、対応する @<tt>{end} に、@<tt>{loop} ブロックではブロックの先頭に飛びます。


==== @<tt>{br}


@<tt>{relative_depth} のイメージ


//emlist{
block ;; 2
  block ;; 1
    block ;; 0
      br 1 ;; 現在のブロックからの深さを指定して分岐
    end
  end
end
//}


blockが値をpushしない場合、オペランドを受け取りません。
blockが値をpushする場合、@<tt>{op1}をpushします。


==== @<tt>{br_if}


blockが値をpushしない場合、@<tt>{op1 != 0} のとき分岐します。
blockが値をpushする場合、@<tt>{op2 != 0} のとき分岐して、@<tt>{op1} をpushします。


==== @<tt>{br_table}


@<tt>{br_table}オペコードは以下のような即値オペランドを持っています。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl34][]{
フィールド	型	説明
-----------------
target_count	@<tt>{varuint32}	target_tableのエントリー数
target_table	@<tt>{varuint32*}	分岐する対象の親ブロック列
default_target	@<tt>{varuint32}	デフォルトで分岐するブロック
//}


ブロックが値をpushしない場合、@<tt>{target_table}の@<tt>{op1}番目のブロックに対して分岐命令を行います。ブロックが値をpushする場合は@<tt>{br_if}などと同様に最初のオペランドがpushされます。入力値が範囲外の場合、@<tt>{br_table}は@<tt>{default_target}に指定されているブロックに対して分岐命令を行います。



//note{
  オペコードの開いてる場所は将来のために予約されてます。
//}

=== Call operators

//tsize[|latex||p{21mm}|p{15mm}|p{37mm}|p{59mm}|]
//table[tbl35][]{
名前	オペコード	即値	説明
-----------------
@<tt>{call}	@<tt>{0x10}	function_index: @<tt>{varuint32}	function indexに対応する関数を呼ぶ
@<tt>{call_indirect}	@<tt>{0x11}	type_index: @<tt>{varuint32}, reserved: @<tt>{veruint1}	@<tt>{type_index} の型の関数を間接的に呼ぶ
//}


@<tt>{call_indirect}は入力として、関数の引数分のオペランド（@<tt>{type_index}から型を参照して引数の数を決定）とテーブルへのindexを受け取ります。テーブルのindex位置の@<tt>{elem_segment}の中で指定されているfunction indexから関数を参照して呼び出します。@<tt>{reserved}は将来的に使うので予約されてます。MVPでは0です。


=== Parametric operators

//tsize[|latex||l|l|l|p{102mm}|]
//table[tbl36][]{
名前	オペコード	即値	説明
-----------------
@<tt>{drop}	@<tt>{0x1a}	.	スタックの1番上の値をpop
@<tt>{select}	@<tt>{0x1b}	.	@<tt>{op3 != 0 ? op1 : op2}
//}

=== Variable access

//tsize[|latex||l|l|l|p{65mm}|]
//table[tbl37][]{
名前	オペコード	即値	説明
-----------------
@<tt>{get_local}	@<tt>{0x20}	local_index: @<tt>{varuint32}	local_indexで指定されたローカル変数をスタックにpush
@<tt>{set_local}	@<tt>{0x21}	local_index: @<tt>{varuint32}	@<tt>{op1}の値をlocal_indexで指定されたローカル変数にセット
@<tt>{tee_local}	@<tt>{0x21}	local_index: @<tt>{varuint32}	@<tt>{op1}の値をlocal_indexで指定されたローカル変数にセットして、同じ値をpush
@<tt>{get_global}	@<tt>{0x23}	global_index: @<tt>{varuint32}	global_indexで指定されたグローバル変数をスタックにpush
@<tt>{set_global}	@<tt>{0x24}	global_index: @<tt>{varuint32}	@<tt>{op1}の値をglobal_indexで指定されたグローバル変数にセット
//}

=== Memory-related operators

//tsize[|latex||l|l|l|p{67mm}|]
//table[tbl38][]{
名前	オペコード	即値	説明
-----------------
@<tt>{i32.load}	@<tt>{0x28}	@<tt>{memory_immediate}	メモリーのオフセット@<tt>{op1 + memory_immediate.offset}（後述）から32bit整数のデータを読み込んでスタックにpush
@<tt>{i64.load}	@<tt>{0x29}	@<tt>{memory_immediate}	略
@<tt>{f32.load}	@<tt>{0x2a}	@<tt>{memory_immediate}	略
@<tt>{f64.load}	@<tt>{0x2b}	@<tt>{memory_immediate}	略
@<tt>{i32.load8_s}	@<tt>{0x2c}	@<tt>{memory_immediate}	符号あり8bit整数を以下略
@<tt>{i32.load8_u}	@<tt>{0x2d}	@<tt>{memory_immediate}	符号なし8bit整数を以下略
@<tt>{i32.load16_s}	@<tt>{0x2e}	@<tt>{memory_immediate}	略
@<tt>{i32.load16_u}	@<tt>{0x2f}	@<tt>{memory_immediate}	略
@<tt>{i64.load8_s}	@<tt>{0x30}	@<tt>{memory_immediate}	略
@<tt>{i64.load8_u}	@<tt>{0x31}	@<tt>{memory_immediate}	略
@<tt>{i64.load16_s}	@<tt>{0x32}	@<tt>{memory_immediate}	略
@<tt>{i64.load16_u}	@<tt>{0x33}	@<tt>{memory_immediate}	略
@<tt>{i64.load32_s}	@<tt>{0x34}	@<tt>{memory_immediate}	略
@<tt>{i64.load32_u}	@<tt>{0x35}	@<tt>{memory_immediate}	略
@<tt>{i32.store}	@<tt>{0x36}	@<tt>{memory_immediate}	メモリーのオフセット@<tt>{op1 + memory_immediate.offset}（後述）に@<tt>{op2}の値を書き込む
@<tt>{i64.store}	@<tt>{0x37}	@<tt>{memory_immediate}	略
@<tt>{f32.store}	@<tt>{0x38}	@<tt>{memory_immediate}	略
@<tt>{f64.store}	@<tt>{0x39}	@<tt>{memory_immediate}	略
@<tt>{i32.store8}	@<tt>{0x3a}	@<tt>{memory_immediate}	略
@<tt>{i32.store16}	@<tt>{0x3b}	@<tt>{memory_immediate}	略
@<tt>{i64.store8}	@<tt>{0x3c}	@<tt>{memory_immediate}	略
@<tt>{i64.store16}	@<tt>{0x3d}	@<tt>{memory_immediate}	略
@<tt>{i64.store32}	@<tt>{0x3e}	@<tt>{memory_immediate}	略
@<tt>{current_memory}	@<tt>{0x3f}	reserved: @<tt>{varuint1}	メモリーのサイズを問い合わせる
@<tt>{grow_memory}	@<tt>{0x40}	reserved: @<tt>{varuint1}	メモリーのサイズを拡張する
//}

@<tt>{memory_immediate} は以下のようにエンコードされます。

//tsize[|latex||p{25mm}|p{25mm}|p{86mm}|]
//table[tbl39][]{
名前	型	説明
-----------------
flags	@<tt>{varuint32}	フラグ。現在は最下位bitsがアライメントの値として使われている（@<tt>{log2(alignment)}でエンコードされる）
offset	@<tt>{varuint32}	オフセット値
//}


flagsは@<tt>{log2(alignment)}でエンコードされるので2の累乗である必要があります。追加のバリデーション基準として、アライメントは元々のアライメント以下でないといけません。@<tt>{log(memory-access-size)}より下位のbitsは0である必要があります。これは将来のために予約されてます（例えば、共有メモリーの順序のために必要）。



@<tt>{current_memory}、@<tt>{grow_memory}オペコードの@<tt>{reserved}は将来的に使われる予定です（MVPでは0）。


=== Constants

//tsize[|latex||l|l|l|p{81mm}|]
//table[tbl40][]{
名前	オペコード	即値	説明
-----------------
@<tt>{i32.const}	@<tt>{0x41}	value:@<tt>{varint32}	@<tt>{i32}と解釈される定数値をpush
@<tt>{i64.const}	@<tt>{0x42}	value:@<tt>{varint64}	@<tt>{i64}と解釈される定数値をpush
@<tt>{f32.const}	@<tt>{0x43}	value:@<tt>{uint32}	@<tt>{f32}と解釈される定数値をpush
@<tt>{f64.const}	@<tt>{0x44}	value:@<tt>{uint64}	@<tt>{f64}と解釈される定数値をpush
//}

=== Comparison operators


真の場合は1、偽の場合は0をpushします。

//tsize[|latex||l|l|l|p{98mm}|]
//table[tbl41][]{
名前	オペコード	即値	説明
-----------------
@<tt>{i32.eqz}	@<tt>{0x45}	.	@<tt>{op1 == 0}
@<tt>{i32.eq}	@<tt>{0x46}	.	@<tt>{op1 == op2}（符号に依存しない）
@<tt>{i32.ne}	@<tt>{0x47}	.	@<tt>{op1 != op2}（符号に依存しない）
@<tt>{i32.lt_s}	@<tt>{0x48}	.	@<tt>{op1 < op2}（符号付き整数として）
@<tt>{i32.lt_u}	@<tt>{0x49}	.	@<tt>{op1 < op2}（符号なし整数として）
@<tt>{i32.gt_s}	@<tt>{0x4a}	.	@<tt>{op1 > op2}（符号付き整数として）
@<tt>{i32.gt_u}	@<tt>{0x4b}	.	@<tt>{op1 > op2}（符号なし整数として）
@<tt>{i32.le_s}	@<tt>{0x4c}	.	@<tt>{op1 <= op2}（符号付き整数として）
@<tt>{i32.le_u}	@<tt>{0x4d}	.	@<tt>{op1 <= op2}（符号なし整数として）
@<tt>{i32.ge_s}	@<tt>{0x4e}	.	@<tt>{op1 >= op2}（符号付き整数として）
@<tt>{i32.ge_u}	@<tt>{0x4f}	.	@<tt>{op1 >= op2}（符号なし整数として）
@<tt>{i64.eqz}	@<tt>{0x50}	.	@<tt>{op1 == 0}
@<tt>{i64.eq}	@<tt>{0x51}	.	@<tt>{op1 == op2}（符号に依存しない）
@<tt>{i64.ne}	@<tt>{0x52}	.	@<tt>{op1 != op2}（符号に依存しない）
@<tt>{i64.lt_s}	@<tt>{0x53}	.	@<tt>{op1 < op2}（符号付き整数として）
@<tt>{i64.lt_u}	@<tt>{0x54}	.	@<tt>{op1 < op2}（符号なし整数として）
@<tt>{i64.gt_s}	@<tt>{0x55}	.	@<tt>{op1 > op2}（符号付き整数として）
@<tt>{i64.gt_u}	@<tt>{0x56}	.	@<tt>{op1 > op2}（符号なし整数として）
@<tt>{i64.le_s}	@<tt>{0x57}	.	@<tt>{op1 <= op2}（符号付き整数として）
@<tt>{i64.le_u}	@<tt>{0x58}	.	@<tt>{op1 <= op2}（符号なし整数として）
@<tt>{i64.ge_s}	@<tt>{0x59}	.	@<tt>{op1 >= op2}（符号付き整数として）
@<tt>{i64.ge_u}	@<tt>{0x5a}	.	@<tt>{op1 >= op2}（符号なし整数として）
@<tt>{f32.eq}	@<tt>{0x5b}	.	@<tt>{op1 == op2}
@<tt>{f32.ne}	@<tt>{0x5c}	.	@<tt>{op1 != op2}
@<tt>{f32.lt}	@<tt>{0x5d}	.	@<tt>{op1 < op2}
@<tt>{f32.gt}	@<tt>{0x5e}	.	@<tt>{op1 > op2}
@<tt>{f32.le}	@<tt>{0x5f}	.	@<tt>{op1 <= op2}
@<tt>{f32.ge}	@<tt>{0x60}	.	@<tt>{op1 >= op2}
@<tt>{f64.eq}	@<tt>{0x61}	.	@<tt>{op1 == op2}
@<tt>{f64.ne}	@<tt>{0x62}	.	@<tt>{op1 != op2}
@<tt>{f64.lt}	@<tt>{0x63}	.	@<tt>{op1 < op2}
@<tt>{f64.gt}	@<tt>{0x64}	.	@<tt>{op1 > op2}
@<tt>{f64.le}	@<tt>{0x65}	.	@<tt>{op1 <= op2}
@<tt>{f64.ge}	@<tt>{0x66}	.	@<tt>{op1 >= op2}
//}

=== Numeric operators

//tsize[|latex||l|l|l|p{95mm}|]
//table[tbl42][]{
名前	オペコード	即値	説明
-----------------
@<tt>{i32.clz}	@<tt>{0x67}	.	@<tt>{op1}の値について最上位bitから0が連続する回数（符号に依存しない）
@<tt>{i32.ctz}	@<tt>{0x68}	.	@<tt>{op1}の値について最下位bitから0が連続する回数（符号に依存しない）
@<tt>{i32.popcnt}	@<tt>{0x69}	.	@<tt>{op1}の値について1になっているbitの数（符号に依存しない）
@<tt>{i32.add}	@<tt>{0x6a}	.	@<tt>{op1 + op2}（符号に依存しない）
@<tt>{i32.sub}	@<tt>{0x6b}	.	@<tt>{op1 - op2}（符号に依存しない）
@<tt>{i32.mul}	@<tt>{0x6c}	.	@<tt>{op1 * op2}（符号に依存しない, 下位32bitが結果になる）
@<tt>{i32.div_s}	@<tt>{0x6d}	.	@<tt>{op1 / op2}符号あり（結果は0方向に切り捨て）
@<tt>{i32.div_u}	@<tt>{0x6e}	.	@<tt>{op1 / op2}符号なし（切り捨て）
@<tt>{i32.rem_s}	@<tt>{0x6f}	.	@<tt>{op1 % op2}符号付き（符号は割られる方が使われる）
@<tt>{i32.rem_u}	@<tt>{0x70}	.	@<tt>{op1 % op2}符号なし
@<tt>{i32.and}	@<tt>{0x71}	.	@<tt>{op1 & op2}（符号に依存しない）
@<tt>{i32.or}	@<tt>{0x72}	.	@<tt>{op1 | op2}（符号に依存しない）
@<tt>{i32.xor}	@<tt>{0x73}	.	@<tt>{op1 ^ op2}（符号に依存しない）
@<tt>{i32.shl}	@<tt>{0x74}	.	@<tt>{op1 << op2}（符号に依存しない）
@<tt>{i32.shr_s}	@<tt>{0x75}	.	@<tt>{op1 >> op2}（算術シフト）
@<tt>{i32.shr_u}	@<tt>{0x76}	.	@<tt>{op1 >>> op2}（論理シフト）
@<tt>{i32.rotl}	@<tt>{0x77}	.	左ローテート（符号に依存しない）
@<tt>{i32.rotr}	@<tt>{0x78}	.	右ローテート（符号に依存しない）
@<tt>{i64.clz}	@<tt>{0x79}	.	最上位bitから0が連続する回数（符号に依存しない）
@<tt>{i64.ctz}	@<tt>{0x7a}	.	最下位bitから0が連続する回数（符号に依存しない）
@<tt>{i64.popcnt}	@<tt>{0x7b}	.	1になっているbitの数（符号に依存しない）
@<tt>{i64.add}	@<tt>{0x7c}	.	@<tt>{op1 + op2}（符号に依存しない）
@<tt>{i64.sub}	@<tt>{0x7d}	.	@<tt>{op1 - op2}（符号に依存しない）
@<tt>{i64.mul}	@<tt>{0x7e}	.	@<tt>{op1 * op2}（符号に依存しない, 下位64bitが結果になる）
@<tt>{i64.div_s}	@<tt>{0x7f}	.	@<tt>{op1 / op2}符号あり（結果は0方向に切り捨て）
@<tt>{i64.div_u}	@<tt>{0x80}	.	@<tt>{op1 / op2}符号なし（切り捨て）
@<tt>{i64.rem_s}	@<tt>{0x81}	.	@<tt>{op1 % op2}符号付き（符号は割られる方が使われる）
@<tt>{i64.rem_u}	@<tt>{0x82}	.	@<tt>{op1 % op2}符号なし
@<tt>{i64.and}	@<tt>{0x83}	.	@<tt>{op1 & op2}（符号に依存しない）
@<tt>{i64.or}	@<tt>{0x84}	.	@<tt>{op1 | op2}（符号に依存しない）
@<tt>{i64.xor}	@<tt>{0x85}	.	@<tt>{op1 ^ op2}（符号に依存しない）
@<tt>{i64.shl}	@<tt>{0x86}	.	@<tt>{op1 << op2}（符号に依存しない）
@<tt>{i64.shr_s}	@<tt>{0x87}	.	@<tt>{op1 >> op2}（算術シフト）
@<tt>{i64.shr_u}	@<tt>{0x88}	.	@<tt>{op1 >>> op2}（論理シフト）
@<tt>{i64.rotl}	@<tt>{0x89}	.	左ローテート（符号に依存しない）
@<tt>{i64.rotr}	@<tt>{0x8a}	.	右ローテート（符号に依存しない）
@<tt>{f32.abs}	@<tt>{0x8b}	.	絶対値
@<tt>{f32.neg}	@<tt>{0x8c}	.	符号逆転
@<tt>{f32.ceil}	@<tt>{0x8d}	.	切り上げ
@<tt>{f32.floor}	@<tt>{0x8e}	.	切り捨て
@<tt>{f32.trunc}	@<tt>{0x8f}	.	0方向に丸める
@<tt>{f32.nearest}	@<tt>{0x90}	.	四捨五入（偶数丸め）
@<tt>{f32.sqrt}	@<tt>{0x91}	.	平方根
@<tt>{f32.add}	@<tt>{0x92}	.	@<tt>{op1 + op2}
@<tt>{f32.sub}	@<tt>{0x93}	.	@<tt>{op1 - op2}
@<tt>{f32.mul}	@<tt>{0x94}	.	@<tt>{op1 * op2}
@<tt>{f32.div}	@<tt>{0x95}	.	@<tt>{op1 / op2}
@<tt>{f32.min}	@<tt>{0x96}	.	2つの値のうち小さい方をpush。どちらかのオペランドがNaNの場合NaN
@<tt>{f32.max}	@<tt>{0x97}	.	2つの値のうち大きい方をpush。どちらかのオペランドがNaNの場合NaN
@<tt>{f32.copysign}	@<tt>{0x98}	.	1つ目の値の符号を2つ目の値にコピーしたものをpush
@<tt>{f64.abs}	@<tt>{0x99}	.	絶対値
@<tt>{f64.neg}	@<tt>{0x9a}	.	符号逆転
@<tt>{f64.ceil}	@<tt>{0x9b}	.	切り上げ
@<tt>{f64.floor}	@<tt>{0x9c}	.	切り捨て
@<tt>{f64.trunc}	@<tt>{0x9d}	.	0方向に丸める
@<tt>{f64.nearest}	@<tt>{0x9e}	.	四捨五入（偶数丸め）
@<tt>{f64.sqrt}	@<tt>{0x9f}	.	平方根
@<tt>{f64.add}	@<tt>{0xa0}	.	@<tt>{op1 + op2}
@<tt>{f64.sub}	@<tt>{0xa1}	.	@<tt>{op1 - op2}
@<tt>{f64.mul}	@<tt>{0xa2}	.	@<tt>{op1 * op2}
@<tt>{f64.div}	@<tt>{0xa3}	.	@<tt>{op1 / op2}
@<tt>{f64.min}	@<tt>{0xa4}	.	2つの値のうち小さい方をpush。どちらかのオペランドがNaNの場合NaN
@<tt>{f64.max}	@<tt>{0xa5}	.	2つの値のうち大きい方をpush。どちらかのオペランドがNaNの場合NaN
@<tt>{f64.copysign}	@<tt>{0xa6}	.	2つ目の値の符号を1つ目の値にコピーしたものをpush
//}

@<raw>{|latex|\clearpage}

=== Conversions


型変換を行います。



@<tt>{f64.convert_s/i32}では最近接偶数へ丸められます。そして、IEEE 754-2008で定義された無限大、負の無限大にオーバーフローする可能性があります。



浮動小数点数から整数に変換する命令（trunc）の場合、0方向に丸められた整数値となります。オーバーフローするような数値を変換しようとするとエラーとなります。

//tsize[|latex||l|l|l|p{83mm}|]
//table[tbl43][]{
名前	オペコード	即値	説明
-----------------
@<tt>{i32.wrap/i64}	@<tt>{0xa7}	.	64bit整数を受け取って、それの下位32bitをpush
@<tt>{i32.trunc_s/f32}	@<tt>{0xa8}	.	32bit浮動小数点数を受け取って、0方向に丸めた32bitの符号付き整数をpush
@<tt>{i32.trunc_u/f32}	@<tt>{0xa9}	.	32bit浮動小数点数を受け取って、0方向に丸めた32bitの符号なし整数をpush
@<tt>{i32.trunc_s/f64}	@<tt>{0xaa}	.	64bit浮動小数点数を受け取って、0方向に丸めた32bitの符号付き整数をpush
@<tt>{i32.trunc_u/f64}	@<tt>{0xab}	.	64bit浮動小数点数を受け取って、0方向に丸めた32bitの符号なし整数をpush
@<tt>{i64.extend_s/i32}	@<tt>{0xac}	.	32bit整数を64bit符号付き整数に拡張する
@<tt>{i64.extend_u/i32}	@<tt>{0xad}	.	32bit整数を64bit符号なし整数に拡張する
@<tt>{i64.trunc_s/f32}	@<tt>{0xae}	.	32bit浮動小数点数を受け取って、0方向に丸めた64bitの符号付き整数をpush
@<tt>{i64.trunc_u/f32}	@<tt>{0xaf}	.	32bit浮動小数点数を受け取って、0方向に丸めた64bitの符号なし整数をpush
@<tt>{i64.trunc_s/f64}	@<tt>{0xb0}	.	64bit浮動小数点数を受け取って、0方向に丸めた64bitの符号付き整数をpush
@<tt>{i64.trunc_u/f64}	@<tt>{0xb1}	.	64bit浮動小数点数を受け取って、0方向に丸めた64bitの符号なし整数をpush
@<tt>{f32.convert_s/i32}	@<tt>{0xb2}	.	32bit整数を32bit浮動小数点数に変換する（符号ありとして）
@<tt>{f32.convert_u/i32}	@<tt>{0xb3}	.	32bit整数を32bit浮動小数点数に変換する（符号なしとして）
@<tt>{f32.convert_s/i64}	@<tt>{0xb4}	.	64bit整数を32bit浮動小数点数に変換する（符号ありとして）
@<tt>{f32.convert_u/i64}	@<tt>{0xb5}	.	64bit整数を32bit浮動小数点数に変換する（符号なしとして）
@<tt>{f32.demote/f64}	@<tt>{0xb6}	.	64bit浮動小数点数から32bit浮動小数点数に変換
@<tt>{f64.convert_s/i32}	@<tt>{0xb7}	.	32bit整数を64bit浮動小数点数に変換する（符号ありとして）
@<tt>{f64.convert_u/i32}	@<tt>{0xb8}	.	32bit整数を64bit浮動小数点数に変換する（符号なしとして）
@<tt>{f64.convert_s/i64}	@<tt>{0xb9}	.	64bit整数を64bit浮動小数点数に変換する（符号ありとして）
@<tt>{f64.convert_u/i64}	@<tt>{0xba}	.	64bit整数を64bit浮動小数点数に変換する（符号なしとして）
@<tt>{f64.promote/f32}	@<tt>{0xbb}	.	32bit浮動小数点数から64bit浮動小数点数に変換
//}

@<raw>{|latex|\clearpage}

=== Reinterpretations


変換前の数値をただのbit列として扱って変換後の型として再解釈します。

//tsize[|latex||l|l|l|p{80mm}|]
//table[tbl44][]{
名前	オペコード	即値	説明
-----------------
@<tt>{i32.reinterpret/f32}	@<tt>{0xbc}	.	32bit浮動小数点数を32bit整数として再解釈する
@<tt>{i64.reinterpret/f64}	@<tt>{0xbd}	.	64bit浮動小数点数を64bit整数として再解釈する
@<tt>{f32.reinterpret/i32}	@<tt>{0xbe}	.	32bit整数を32bit浮動小数点数として再解釈する
@<tt>{f64.reinterpret/i64}	@<tt>{0xbf}	.	64bit整数を64bit浮動小数点数として再解釈する
//}

@<raw>{|latex|\clearpage}

　
