# Binary Format

## Data types

### Numbers

#### `uintN`
符号なしの *N* bitの整数値。リトルエンディアン。
`uint8`, `uint16`, `uint32`の3種類が使用されています。

#### `veruintN`
[LEB128](https://en.wikipedia.org/wiki/LEB128)で表現される *N* bitの符号なし整数。

Note:
現在は `varuint1`, `veruint7`, `varuint32`のみ使用されてます。
前者2つは将来的な拡張機能と互換性のために使用されています(LEB128だと後で伸ばせるのでこれを使っているんだと思う)。

#### `verintN`
符号付きのLEB128で表現される *N* bitの整数。
Note:
現在は`varint7`, `varint32`, `varint64`が使用されています。

### Language Types

全ての型は(型コンストラクタを表す)先頭の負の`varint7`値によって識別されます。

Opcode(`varint7`値) | Opcode(バイト値)  | Type constructor
:---|:---|:---
`-0x01`|`0x7f`|`i32`
`-0x02`|`0x7e`|`i64`
`-0x03`|`0x7d`|`f32`
`-0x04`|`0x7c`|`f64`
`-0x10`|`0x70`|`anyfunc`(何らかの関数)
`-0x20`|`0x60`|`func`
`-0x40`|`0x40`|空の`block_type`を表現する擬似的な型

これらのうち、いくつかは追加のフィールドが続きます(後述)。

Note:
:unicorn:将来的な拡張性のために隙間(`-0x05`とか)の数値は予約されています。符号付きの数値(つまりここで負の値)を使っているは1バイトでtype sectionへのインデックス(後述)を張ることもできるようにするためで、型システムの将来的な拡張に関連しています。

#### `value_type`
値型を示す`varint7`値

* `i32`
* `i64`
* `f32`
* `f64`

のうちの一つ。上の表を参照(上4つ)。

#### `block_type`
ブロックが返す型を示す`varint7`値。上の`value_type`(1個の値を返す)か、`-0x40`(0個の値を返す)です。

#### `elem_type`
table(後述)中の要素の型を示す`varint7`値。MVPでは以下の型のみ有効です。

* `anyfunc`

Note: :unicorn:将来的に他の要素も有効になるかも。

#### `func_type`
関数シグネチャー。この型コンストラクタには以下の記述が追加されます。

Field | Type | Description
:---|:---|:---
form | `varint7` | 上の表で定義した`func`型コンストラクタの値
param_count | varuint32 | 関数のパラメータ数
param_types | `value_type*` | 関数のパラメータの型(パラメータ数だけ用意)
return_count | `varuint1` | 関数の返り値の数
return_type | `value_type?` | 関数の返り値の型(return_countが1なら)

Note:
:unicorn:`return_count`と`return_type`は複数の値を許容するために一般化するかもしれません。

### Other Types

#### `global_type`
グローバル変数

Field | Type | Description
:---|:---|:---
content_type | `value_type` | 値の型
mutability | `varuint1` | `0`ならイミュータブル。`1`ならミュータブル

#### `table_type`
テーブル

Field | Type | Description
:---|:---|:---
element_type | `elem_type` | 要素の型
limits | `resizable_limits` | 後述

#### `memory_type`
メモリー

Field | Type | Description
:---|:---|:---
limits | `resizable_limits` | 後述

#### `external_kind`
インポートまたはモジュール内で定義された定義の種類を示す1バイトの符号なし整数値。

* `0`: インポートまたは定義された `Function`
* `1`: インポートまたは定義された `Table`
* `2`: インポートまたは定義された `Memory`
* `3`: インポートまたは定義された `Global`

#### `resizable_limits`
tableかmemoryのリミットを記述するタプル。

Field | Type | Description
:---|:---|:---
flags | `varuint1` | maximumフィールドがあるかどうか
initial | `varuint32` | 初期の長さ(テーブルなら要素数、メモリーならページ(64KB)を1単位とする)
maximum | `varuint32?` | 最大値(`flags`が`1`ならこのフィールドがある)

Note:
:unicorn: `flags` フィールドは`varuint32`に変わる可能性があります。例えばスレッド間で共有するためのフラグが含まれたりする。

#### `init_expr`
イニシャライザー。code section(後述)の式部分+`end`オペコード。

Note:
イニシャライザーに含まれる`get_global`はインポートされたイミュータブルなグローバル変数のみ参照でき、全ての`init_expr`はimport sectionの後でのみ使用できます。

### Module structure

以下のドキュメントはwasmモジュールの現在のプロトタイプです。このフォーマットは[v8-native prototype format](https://docs.google.com/document/d/1-G11CnMA0My20KI9D7dBR6ZCPOBCRD0oCH6SHCPFGx0/edit)を元にして作られてて、取って代わるものとなります。

## High-level structure
モジュールは以下の2つのフィールド(プリアンブルっていうんだって)から開始します。

Field | Type | Description
:---|:---|:---
magic number | `uint32` | マジックナンバー `0x6d736100` (`"\0asm"`)
version | `uint32` | バージョンナンバー。現状は `0x0d`。このMVP用のバージョンは1にリセットされる予定です。

モジュールプリアンブルのあとに一連のセクションが続きます。各セクションは既知のセクションまたはカスタムセクションを表す1バイトのセクションコードによって識別されます。そして、セクションの長さ、ペイロードが次に続きます。既知のセクションは非ゼロなidを持ちます。カスタムセクションのidは`0`で、その後にペイロードの一部として識別文字列が続きます。カスタムセクションはWebAssemblyの実装では無視されるので内部でのバリデーションエラーがあってもモジュールが無効になるわけではありません。

Field | Type | Description
:---|:---|:---
id | `varuint7` | セクションコード
payload_len | `varuint32` | セクションのバイト長
name_len | `varuint32?` | セクション名のバイト長。`id == 0`のときだけ存在します
name | `bytes?` | セクション名の本体。`id == 0`のときだけ存在します
payload_data | セクションの中身。長さは `payload_len - sizeof(name) - sizeof(name_len)`。

既知のセクションはオプショナルで最大で1個だけです。カスタムセクションは全て同じidを持ちユニークでない名前をつけることができます。以下の表で定義される既知のセクションは順不同で現れない場合もあります。各セクションの`payload_data`にエンコードされた内容が入ります。

Section Name | Code | Description
:---|:---|:---
Type | `1` | 関数シグネチャー宣言
Import | `2` | インポート宣言
Function | `3` | 関数宣言
Table | `4` | 間接的な関数テーブルと他のテーブル
Memory | `5` | メモリー属性
Global | `6` | グローバル宣言
Export | `7` | エクスポート
Start | `8` | 開始関数宣言
Element | `9` | 要素セクション
Code | `10` | 関数本体(code)
Data | `11` | データセグメント

最後のセクションの最終バイトはモジュールの最終バイトと一致する必要があります。最小のモジュールは8バイトです (`magic number`, `version`のあとに何もない状態)。

### Type section

type sectionでは、モジュールで使用されている全ての関数シグネチャーを宣言します。

Field | Type | Description
:---|:---|:---
count | `varuint32` | エントリー数
entries | `func_type*` | 関数シグネチャー列

Note:
:unicorn:将来的に、他の型を持つエントリーもここに入る可能性があります。`func_type`の`form`フィールドで識別できます。

### Import section
import sectionではモジュールで使用されている全てのインポートを宣言します。

Field | Type | Description
:---|:---|:---
count | `varuint32` | エントリー数
entries | `import_entry*` | import entry(後述)列

#### Import entry

Field | Type | Description
:---|:---|:---
module_len | `varuint32` | モジュール文字列の長さ
module_str | `bytes` | `module_len`バイトのモジュールの文字列
field_len | `varuint32` | フィールド名の長さ
field_str | `bytes` | `field_len`バイトのフィールド名
kind | `external_kind` | インポートされた定義の種類

`kind`が`Function`の場合、以下の項目が続く。

Field | Type | Description
:---|:---|:---
type | `varuint32` | type sectionにある関数シグネチャーのインデックス

or `kind`が`Table`の場合は

Field | Type | Description
:---|:---|:---
type | `table_type` | インポートされたテーブルの型

or `kind`が`Memory`の場合は

Field | Type | Description
:---|:---|:---
type | `memory_type` | インポートされたメモリーの型

or `kind`が`Global`の場合は

Field | Type | Description
:---|:---|:---
type | `global_type` | インポートされたグローバル変数の型

Note: MVP(minimum viable product)では イミュータブルなグローバル変数のみインポートできます。

### Function section

Function sectionではモジュール内のすべての関数シグネチャーを宣言します(関数の定義はcode sectionに置かれます)。

Field | Type | Description
:---|:---|:---
count | `varuint32` | エントリー数
types | `varuint32*` | type sectionに置いてある対象の`func_type`のインデックス列

### Table section

Field | Type | Description
:---|:---|:---
count | `varuint32` | モジュールに定義されているテーブルの数
entries | `table_type*` | `table_type`エントリー列

型付き配列のリストと言った感じです。MVPでは、テーブルの数は1以下でなければいけません。`table_type`で使用できる型がanyfuncだけで、これは関数テーブルとして機能します。

### Memory section

Field | Type | Description
:---|:---|:---
count | `varuint32` | モジュールで定義されているメモリーの数
entries | `memory_type*` | `memory_type` エントリー列

Note: 初期値、最大値のフィールドは[WebAssembly pages](http://webassembly.org/docs/semantics/#linear-memory)参照。
MVPでは、メモリーの数は必ず1以下でなければいけません。

### Global section

Field | Type | Description
:---|:---|:---
count | `varuint32` | グローバル変数エントリーの数
globals | `global_variable*` | グローバル変数たち(後述)

#### `global_variable`

各`global_variable`は`type`(下の表)で与えられた型のグローバル変数の宣言です。イニシャライザー(外から値を与えられる)を持っていてミュータブルです。

Field | Type | Description
:---|:---|:---
type | `global_type` | 変数の型
init | `init_expr` | グローバル変数のイニシャライザー

Note:
MVPではイミュータブルなグローバル変数だけエクスポートできます。

### Export section

Field | Type | Description
:---|:---|:---
count | `varuint32` | エントリー数
entries | `export_entry*` | `export_entry`列(後述)

#### `export_entry`

Field | Type | Description
:---|:---|:---
field_len | `varuint32` | フィールド名の長さ
field_str | `bytes` | `field_len`バイトのフィールド名
kind | `external_kind` | エクスポートされた定義の種類
index | `varuint32` | 対応するindex space(後述)へのインデックス

#### index spaceの補足

例えば、`kind`が`Function`のときはindexはfunction section中でのインデックス(function index)になります。MVPではメモリ、テーブルは0だけです。

### Start section

start functionの宣言をします。

Field | Type | Description
:---|:---|:---
index | `varuint32`|開始する関数のfunction index

### Element section

Field | Type | Description
:---|:---|:---
count| `varuint32`|エレメントセグメントの数
entries|`elem_segment*`|`element_segment`列(後述)

関数テーブルにfunction indexをつっこむセクション。

#### `elem_segment`

Field | Type | Description
:---|:---|:---
index| `varuint32`|table index(MVPでは0)
offset| `init_expr`|要素の場所のオフセットを返すイニシャライザー(`i32`の値が返ってくる)
num_elem|`varuint32`|要素数
elems|`varuint32*`|function index列

### Code section

code sectionにはモジュール内の全ての関数の本体が含まれています。function sectionで宣言された関数の数と関数本体の定義は必ず一致しなければいけません。`i`番目の関数宣言は`i`番目の関数本体に対応しています。

Field | Type | Description
:---|:---|:---
count|`varuint32`|`function_body`の数.
bodies|`function_body*`|関数本体の列(Function bodiesで説明)

### Data section
[linear memory](http://webassembly.org/docs/semantics/#linear-memory)にロードされる初期化データを宣言します。

Field | Type | Description
:---|:---|:---
count|`varuint32|データセグメントの数
entries|`data_segment*`|`data_segment`列(後述)

#### `data_segment`

Field | Type | Description
:---|:---|:---
index|`varuint32`|リニアメモリのインデックス(MVPでは0です)
offset|`init_expr`|データの場所のオフセットを返すイニシャライザー(`i32`の値が返ってくる)
size|`varuint32`|`data`のバイト数
data|`bytes`|データ本体

### Name section
カスタムセクションに定義される関数名を記録しておくフィールドです。全てのカスタムセクション同様、このセクションでのバリデーションエラーは全体のモジュールには影響しません。
Name sectionはデータセクションのあとに1回だけ現れます。wasmのモジュールやその他開発環境で表示する場合、Name sectionで定義される関数名やローカル変数名はWebAssemblyのテキスト表現などで使用されることが期待されます。

Field | Type | Description
:---|:---|:---
count|`varuint32`|エントリー数
entries|`function_names*`|名前の列

`function_names`の列はfunction indexに対応しています。
(Note: インポートされた関数、モジュール内で定義された関数の両方に名前が割り当てられます)
実際の関数の数より多くても少なくてもかまいません。

#### `function_names`

Field | Type | Description
:---|:---|:---
fun_name_len|`varuint32`|関数名のバイト長
fun_name_str|`bytes`|utf8文字列
local_count|`varuint32`|ローカル変数の名前の数
local_names|`local_name*`|ローカル変数名の列

`local_name`の列はlocal indexに対応しています。実際の関数の数より多くても少なくてもかまいません。

#### `local_name`

Field | Type | Description
:---|:---|:---
local_name_len|`varuint32`|バイト長
local_name_str|`bytes`|validなutf8文字列

## Function Bodies(関数本体)

関数本体は一連のローカル変数宣言の後にバイトコード命令が続きます(いわゆるスタックマシン[Design Rationale \- WebAssembly](http://webassembly.org/docs/rationale/))。
各関数本体は`end`オペコードで終わらなければなりません。

Field | Type | Description
:---|:---|:---
body_size|`varuint32`|関数本体のバイト数
local_count|`varuint32`|ローカル変数の数
locals|`local_entry`|ローカル変数列
code|`btes*`|関数のバイトコード
end|`byte|`0x0b`終了を示す

### Local entry

各ローカルエントリーは与えられた型のローカル変数リストを宣言します。同じ型のエントリーを複数持つのはokです。

Field | Type | Description
:---|:---|:---
count |`varuint32`|ローカル変数の数
type|`value_type`|変数の型

### Control flow operators

全体的になんですが、immediatesは即値オペランドです。オペコードの直後に置かれます。
オペコードがオペランドを受け取る場合は基本的に1個目から`x`, `y`, `z`としておきます。

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`unreachable`|`0x00`||即例外を発生
`nop`|`0x01`||なにもしないオペレータ
`block`|`0x02`|sig:`block_type`|式のシーケンスを開始して、0or1個の値をpush
`loop`|`0x03`|sig:`block_type`|ループできるブロックを作る
`if`|`0x04`|sig:`block_type`|if式。`x != 0`のときブロックに入る
`else`|`0x05`||ifに対応するelse式
`end`|`0x0b`||`block`、`if`、`loop`の終了
`br`|`0x0c`|relative_depth:`varuint32`|対象の親ブロックをブレーク(以下補足)
`br_if`|`0x0d`|relative_depth:`varuint32`|`br`の条件分岐あり版(以下補足)
`br_table`|`0x0e`|後述|後述
`return`|`0x0f`||関数から0個か1個の値を返す

#### `br`

*relative_depth* のイメージ

```
block ;; 2
  block ;; 1
    block ;; 0
      br 1 ;; 現在のブロックからの深さを指定してブレーク
    end
  end
end
```

blockが値をpushしない場合、オペランドを受け取りません。
blockが値をpushする場合、`x`をpush。

#### `br_if`

blockが値をpushしない場合、`x == 1`のときブレークします。
blockが値をpushする場合、`y == 1`のときブレークして、`x`をpush。

#### `br_table`

`br_table`オペレータは以下のような即値オペランドを持っています。

Field | Type | Description
:---|:---|:---
target_count | `varuint32` | target_tableのエントリー数
target_table|`varuint32*`|ブレークする対象の親ブロック列
default_target|`varuint32`|デフォルトでブレークするブロック

blockが値を返さない場合、`target_table`の`x`番目のブロックをブレークします。blockが値をpushする場合は`br_if`とかと同様に最初のオペランドがpushされます。入力値が範囲外の場合、`br_table`は`default_target`に指定されているブロックをブレークします。

Note: :unicorn:opcodeの開いてる場所は将来のために予約されてます。

### Call operators

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`call`|`0x10`|function_index: `varuint32`| 与えられたindexの関数を呼ぶ
`call_indirect`|`0x11`|type_index: `varuint32`, reserved: `veruint1`| *type_index* の型の関数を間接的に呼ぶ

`call_indirect`は入力として、関数の引数分のオペランド(`type_index`から型を参照して引数の数を決定)+テーブルへのindexを受け取ります。テーブルのindexに置いてある`elem_segment`で指定されているfunction indexから関数を参照して呼び出します。
`reserved`は将来的に使うので予約されてます。MVPでは0です。

### Parametric operators

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`drop`|`0x1a`||スタックの1番上の値をpop
`select`|`0x1b`|| `z != 0 ? x : y`

# Variable access

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`get_local`|`0x20`|local_index: `varuint32`|ローカル変数or引数を読みこむ
`set_local`|`0x21`|local_index: `varuint32`|ローカル変数or引数を書きこむ
`tee_local`|`0x21`|local_index: `varuint32`|ローカル変数or引数を書きこむ、そして同じ値をpush
`get_global`|`0x23`|global_index: `varuint32`|グローバル変数を読みこむ
`set_global`|`0x24`|global_index: `varuint32`|グローバル変数を書きこむ

# Memory-related operators

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`i32.load`|`0x28`|`memory_immediate`|メモリーからよみこむ
`i64.load`|`0x29`|`memory_immediate`|メモリーからよみこむ
`f32.load`|`0x2a`|`memory_immediate`|メモリーからよみこむ
`f64.load`|`0x2b`|`memory_immediate`|メモリーからよみこむ
`i32.load8_s`|`0x2c`|`memory_immediate`|メモリーからよみこむ
`i32.load8_u`|`0x2d`|`memory_immediate`|メモリーからよみこむ
`i32.load16_s`|`0x2e`|`memory_immediate`|メモリーからよみこむ
`i32.load16_u`|`0x2f`|`memory_immediate`|メモリーからよみこむ
`i64.load8_s`|`0x30`|`memory_immediate`|メモリーからよみこむ
`i64.load8_u`|`0x31`|`memory_immediate`|メモリーからよみこむ
`i64.load16_s`|`0x32`|`memory_immediate`|メモリーからよみこむ
`i64.load16_u`|`0x33`|`memory_immediate`|メモリーからよみこむ
`i64.load32_s`|`0x34`|`memory_immediate`|メモリーからよみこむ
`i64.load32_u`|`0x35`|`memory_immediate`|メモリーからよみこむ
`i32.store`|`0x36`|`memory_immediate`|メモリーにかきこむ
`i64.store`|`0x37`|`memory_immediate`|メモリーにかきこむ
`f32.store`|`0x38`|`memory_immediate`|メモリーにかきこむ
`f64.store`|`0x39`|`memory_immediate`|メモリーにかきこむ
`i32.store8`|`0x3a`|`memory_immediate`|メモリーにかきこむ
`i32.store16`|`0x3b`|`memory_immediate`|メモリーにかきこむ
`i64.store8`|`0x3c`|`memory_immediate`|メモリーにかきこむ
`i64.store16`|`0x3d`|`memory_immediate`|メモリーにかきこむ
`i64.store32`|`0x3e`|`memory_immediate`|メモリーにかきこむ
`current_memory`|`0x3f`|reserved: `varuint1`|メモリーのサイズを問い合わせる
`grow_memory`|`0x40`|reserved: `varuint1`|メモリーのサイズを拡張する

`memory_immediate` は以下のようにエンコードされます。

Name | Type | Description
:---|:---|:---
flags | `varuint32` | フラグ。現在は最下位bitsがアライメントの値として使われている(`log2(alignment)`でエンコードされる)
offset | `varuint32` | オフセット値

flagsは`log2(alignment)`でエンコードされるので2の累乗である必要があります。追加のバリデーション基準として、アライメントは自然アライメント以下でないといけません。最下位bitsより後ろの`log(memory-access-size)`bitsは0である必要があります。将来のために予約されてます(例えば、共有メモリーの順序のために必要)。
`current_memory` `grow_memory`オペレータの`reserved`即値は将来的に使われるかも。MVPでは0です。

## Constants

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`i32.const`|`0x41`|value:`varint32`|`i32`と解釈される定数値
`i64.const`|`0x42`|value:`varint64`|`i64`と解釈される定数値
`f32.const`|`0x43`|value:`uint32`|`f32`と解釈される定数値
`f64.const`|`0x44`|value:`uint64`|`f64`と解釈される定数値

## Comparison operators

真の場合は1、偽の場合は0を返します。

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`i32.eqz`|`0x45`||`x == 0`
`i32.eq`|`0x46`||`x == y`(符号に依存しない)
`i32.ne`|`0x47`||`x != y`(符号に依存しない)
`i32.lt_s`|`0x48`||`x < y`(符号付き整数として)
`i32.lt_u`|`0x49`||`x < y`(符号なし整数として)
`i32.gt_s`|`0x4a`||`x > y`(符号付き整数として)
`i32.gt_u`|`0x4b`||`x > y`(符号なし整数として)
`i32.le_s`|`0x4c`||`x <= y`(符号付き整数として)
`i32.le_u`|`0x4d`||`x <= y`(符号なし整数として)
`i32.ge_s`|`0x4e`||`x >= y`(符号付き整数として)
`i32.ge_u`|`0x4f`||`x >= y`(符号なし整数として)
`i64.eqz`|`0x50`||`x == 0`
`i64.eq`|`0x51`||`x == y`(符号に依存しない)
`i64.ne`|`0x52`||`x != y`(符号に依存しない)
`i64.lt_s`|`0x53`||`x < y`(符号付き整数として)
`i64.lt_u`|`0x54`||`x < y`(符号なし整数として)
`i64.gt_s`|`0x55`||`x > y`(符号付き整数として)
`i64.gt_u`|`0x56`||`x > y`(符号なし整数として)
`i64.le_s`|`0x57`||`x <= y`(符号付き整数として)
`i64.le_u`|`0x58`||`x <= y`(符号なし整数として)
`i64.ge_s`|`0x59`||`x >= y`(符号付き整数として)
`i64.ge_u`|`0x5a`||`x >= y`(符号なし整数として)
`f32.eq`|`0x5b`||`x == y`
`f32.ne`|`0x5c`||`x != y`
`f32.lt`|`0x5d`||`x < y`
`f32.gt`|`0x5e`||`x > y`
`f32.le`|`0x5f`||`x <= y`
`f32.ge`|`0x60`||`x >= y`
`f64.eq`|`0x61`||`x == y`
`f64.ne`|`0x62`||`x != y`
`f64.lt`|`0x63`||`x < y`
`f64.gt`|`0x64`||`x > y`
`f64.le`|`0x65`||`x <= y`
`f64.ge`|`0x66`||`x >= y`

## Numeric operators

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`i32.clz`|`0x67`||最上位bitから0が連続する回数(符号に依存しない)
`i32.ctz`|`0x68`||最下位bitから0が連続する回数(符号に依存しない)
`i32.popcnt`|`0x69`||1になっているbitの数(符号に依存しない)
`i32.add`|`0x6a`||`x + y`(符号に依存しない)
`i32.sub`|`0x6b`||`x - y`(符号に依存しない)
`i32.mul`|`0x6c`||`x * y`(符号に依存しない, 下位32bitが結果になる)
`i32.div_s`|`0x6d`||`x / y`符号あり(結果は0方向に切り捨て)
`i32.div_u`|`0x6e`||`x / y`符号なし(切り捨て)
`i32.rem_s`|`0x6f`||`x % y`符号付き(符号は割られる方が使われる)
`i32.rem_u`|`0x70`||`x % y`符号なし
`i32.and`|`0x71`||`x & y`(符号に依存しない)
`i32.or`|`0x72`||`x | y`(符号に依存しない)
`i32.xor`|`0x73`||`x ^ y`(符号に依存しない)
`i32.shl`|`0x74`||`x << y`(符号に依存しない)
`i32.shr_s`|`0x75`||`x >> y`(算術シフト)
`i32.shr_u`|`0x76`||`x >>> y`(論理シフト)
`i32.rotl`|`0x77`||左ローテート(符号に依存しない)
`i32.rotr`|`0x78`||右ローテート(符号に依存しない)
`i64.clz`|`0x79`||最上位bitから0が連続する回数(符号に依存しない)
`i64.ctz`|`0x7a`||最下位bitから0が連続する回数(符号に依存しない)
`i64.popcnt`|`0x7b`||1になっているbitの数(符号に依存しない)
`i64.add`|`0x7c`||`x + y`(符号に依存しない)
`i64.sub`|`0x7d`||`x - y`(符号に依存しない)
`i64.mul`|`0x7e`||`x * y`(符号に依存しない, 下位64bitが結果になる)
`i64.div_s`|`0x7f`||`x / y`符号あり(結果は0方向に切り捨て)
`i64.div_u`|`0x80`||`x / y`符号なし(切り捨て)
`i64.rem_s`|`0x81`||`x % y`符号付き(符号は割られる方が使われる)
`i64.rem_u`|`0x82`||`x % y`符号なし
`i64.and`|`0x83`||`x & y`(符号に依存しない)
`i64.or`|`0x84`||`x | y`(符号に依存しない)
`i64.xor`|`0x85`||`x ^ y`(符号に依存しない)
`i64.shl`|`0x86`||`x << y`(符号に依存しない)
`i64.shr_s`|`0x87`||`x >> y`(算術シフト)
`i64.shr_u`|`0x88`||`x >>> y`(論理シフト)
`i64.rotl`|`0x89`||左ローテート(符号に依存しない)
`i64.rotr`|`0x8a`||右ローテート(符号に依存しない)
`f32.abs`|`0x8b`||絶対値
`f32.neg`|`0x8c`||符号逆転
`f32.ceil`|`0x8d`||切り上げ
`f32.floor`|`0x8e`||切り捨て
`f32.trunc`|`0x8f`||0方向に丸める
`f32.nearest`|`0x90`||四捨五入(偶数丸め)
`f32.sqrt`|`0x91`||平方根
`f32.add`|`0x92`||`x + y`
`f32.sub`|`0x93`||`x - y`
`f32.mul`|`0x94`||`x * y`
`f32.div`|`0x95`||`x / y`
`f32.min`|`0x96`||2つの値のうち小さい方をpush。どちらかのオペランドがNaNの場合NaN
`f32.max`|`0x97`||2つの値のうち大きい方をpush。どちらかのオペランドがNaNの場合NaN
`f32.copysign`|`0x98`||1つ目の値の符号を2つ目の値にコピーしたものをpush
`f64.abs`|`0x99`||絶対値
`f64.neg`|`0x9a`||符号逆転
`f64.ceil`|`0x9b`||切り上げ
`f64.floor`|`0x9c`||切り捨て
`f64.trunc`|`0x9d`||0方向に丸める
`f64.nearest`|`0x9e`||四捨五入(偶数丸め)
`f64.sqrt`|`0x9f`||平方根
`f64.add`|`0xa0`||`x + y`
`f64.sub`|`0xa1`||`x - y`
`f64.mul`|`0xa2`||`x * y`
`f64.div`|`0xa3`||`x / y`
`f64.min`|`0xa4`||2つの値のうち小さい方をpush。どちらかのオペランドがNaNの場合NaN
`f64.max`|`0xa5`||2つの値のうち大きい方をpush。どちらかのオペランドがNaNの場合NaN
`f64.copysign`|`0xa6`||2つ目の値の符号を1つ目の値にコピーしたものをpush

## Conversions

型変換系

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`i32.wrap/i64`|`0xa7`||64bit整数を受け取って、それの下位32bitをpush
`i32.trunc_s/f32`|`0xa8`||32bit浮動小数点数を受け取って、0方向に丸めた32bitの符号付き整数をpush(変換後の数値が-2^31-1~2^31の範囲に収まること)
`i32.trunc_u/f32`|`0xa9`||32bit浮動小数点数を受け取って、0方向に丸めた32bitの符号なし整数をpush(変換後の数値が2^32までであること)
`i32.trunc_s/f64`|`0xaa`||64bit浮動小数点数を受け取って、0方向に丸めた32bitの符号付き整数をpush(変換後の数値が-2^31-1~2^31の範囲に収まること)
`i32.trunc_u/f64`|`0xab`||64bit浮動小数点数を受け取って、0方向に丸めた32bitの符号なし整数をpush(変換後の数値が2^32までであること)
`i64.extend_s/i32`|`0xac`||32bit整数を64bit符号付き整数に拡張する
`i64.extend_u/i32`|`0xad`||32bit整数を64bit符号なし整数に拡張する
`i64.trunc_s/f32`|`0xae`||32bit浮動小数点数を受け取って、0方向に丸めた64bitの符号付き整数をpush(変換後の数値が-2^31-1~2^31の範囲に収まること)
`i64.trunc_u/f32`|`0xaf`||32bit浮動小数点数を受け取って、0方向に丸めた64bitの符号なし整数をpush(変換後の数値が2^32までであること)
`i64.trunc_s/f64`|`0xb0`||64bit浮動小数点数を受け取って、0方向に丸めた64bitの符号付き整数をpush(変換後の数値が-2^63-1~2^63の範囲に収まること)
`i64.trunc_u/f64`|`0xb1`||64bit浮動小数点数を受け取って、0方向に丸めた64bitの符号なし整数をpush(変換後の数値が2^64までであること)
`f32.convert_s/i32`|`0xb2`||32bit整数を32bit浮動小数点数に変換する(符号ありとして)
`f32.convert_u/i32`|`0xb3`||32bit整数を32bit浮動小数点数に変換する(符号なしとして)
`f32.convert_s/i64`|`0xb4`||64bit整数を32bit浮動小数点数に変換する(符号ありとして)
`f32.convert_u/i64`|`0xb5`||64bit整数を32bit浮動小数点数に変換する(符号なしとして)
`f32.demote/f64`|`0xb6`||64bit浮動小数点数から32bit浮動小数点数に変換
`f64.convert_s/i32`|`0xb7`||32bit整数を64bit浮動小数点数に変換する(符号ありとして)
`f64.convert_u/i32`|`0xb8`||32bit整数を64bit浮動小数点数に変換する(符号なしとして)
`f64.convert_s/i64`|`0xb9`||64bit整数を64bit浮動小数点数に変換する(符号ありとして)
`f64.convert_u/i64`|`0xba`||64bit整数を64bit浮動小数点数に変換する(符号なしとして)
`f64.promote/f32`|`0xbb`||32bit浮動小数点数から64bit浮動小数点数に変換

## Reinterpretations

変換前の数値をただのbit列として扱って変換後の型として再解釈する

Name | Opcode | Immediates | Description
:---|:---|:---|:---
`i32.reinterpret/f32`|`0xbc`||32bit浮動小数点数を32bit整数として再解釈する
`i64.reinterpret/f64`|`0xbd`||64bit浮動小数点数を64bit整数として再解釈する
`f32.reinterpret/i32`|`0xbe`||32bit整数を32bit浮動小数点数として再解釈する
`f64.reinterpret/i64`|`0xbf`||64bit整数を64bit浮動小数点数として再解釈する

## 参考

[Binary Encoding \- WebAssembly](http://webassembly.org/docs/binary-encoding/)
[Introduction to WebAssembly — Rasmus Andersson](https://rsms.me/wasm-intro)
[LEB128 \- Wikipedia](https://en.wikipedia.org/wiki/LEB128)
[WebAssembly/spec: Staging ground for artifacts related to an MVP spec](https://github.com/WebAssembly/spec)
