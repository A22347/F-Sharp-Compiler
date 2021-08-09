## C/F# to Assembly Compiler

A compiler for a custom language combining C and F#.

```
(*

<match-expression>          ::= match <expression> with <match-type-case-list>
                                match <expression> with <match-case-list>

<match-case>                ::= | _ -> <expression>
                                | <expression> when <expression> -> <expression>
                                | <expression> -> <expression>

<match-case-list>           ::= <match-case> <match-case-list>
                                <match-case>

<match-type-case>           ::= | _ -> <expression>
                                | <type-name> -> <expression>

<match-type-case-list>      ::= <match-type-case> <match-type-case-list>
                                <match-type-case>                          

<declaration-b-list>        ::= <declaration-b> <declaration-b-list>
                                <declaration-b>

<property-statement>        ::= member <identifier> . <identifier> with set ( <identifier> ) = <expression> and get = <expression>
                                member <identifier> . <identifier> with get = <expression> and set ( <identifier> ) = <expression>
                                member <identifier> . <identifier> with get = <expression>
                                member <identifier> . <identifier> with set ( <identifier> )
                                member <identifier> . <identifier> = <expression>

<parameter-list>            ::= ( <declaration-d-list> -> <type-name> )
                                ( <declaration-d-list> )
                                ( <type-name> )
                                ()

<member-statement>          ::= let <identifier> . <identifier> <parameter-list> = <expression> 
                                let <declaration-a> = <expression>
                                <property-statement>

<declaration-d-list>        ::= <declaration-d> <declaration-d-list>
                                <declaration-d>

<declaration-c-list>        ::= <declaration-b> <declaration-c-list>
                                <member-statement> <declaration-c-list>
                                <declaration-b>
                                <member-statement>

<declaration-b>             ::= <identifier> : <type-name> << <type-name> <integer-literal> .. <integer-literal> >>
                                <identifier> : <type-name> << <type-name> >>
                                <identifier> : <type-name>

<declaration-d>             ::= <identifier> : <type-name>

<declaration-a>             ::= <identifier> : <type-name>
                                <identifier>

<type-name-list>            ::= <type-name> , <type-name-list> 
                                <type-name>

<union-case-list>           ::= <union-case> <type-name-list> 
                                <union-case>

<type-name>                 ::= unsigned int
                                unsigned char
                                signed int
                                signed char
                                int
                                char
                                float
                                double
                                u64
                                u32
                                u16
                                u8
                                i64
                                i32
                                i16
                                i8
                                intptr
                                uintptr
                                <type-name> *
                                <type-name> [ <const-expr> ]
                                <type-name> [ ]
                                <identifier>

<type-declaration>          ::= type delegate <identifier> = <type-name-list> -> <type-name>
                                type <identifier> ( <type-name> ) = <union-case-list>
                                type <identifier> = <union-case-list>
                                type <identifier> = <type-name> pointer
                                type <identifier> = <type-name>
                                type <identifier> = { <declaration-c-list> }

<union-case>                ::= | <identifier> = <const-expr> { <declaration-b-list> }
                                | <identifier> { <declaration-b-list> }
                                | <identifier> = <const-expr>
                                | <identifier>

<expression-list>           ::= <expression> <expression-list>                                

<let-statement>             ::= let mutable <declaration-a> = <expression>
                                let <declaration-a> = <expression>
                                let rec <identifier> <parameter-list> = <expression>
                                let <identifier> <parameter-list> = <expression>

<expression-list>           ::= <expression> <expression-list>
                                <expression>

<const-expr-list>           ::= <const-expr> , <const-expr-list>
                                <const-expr> ,
                                <const-expr>

<while-expr>                ::= while <expression> do <expression>

<array-literal-list>        ::= <array-literal>, <array-literal-list>
                                <array-literal> ,
                                <array-literal>

<array-literal>             ::= [ <const-expr-list> ]
                                [ <array-literal-list> ]

<if-expr>                   ::= if <expression> then <expression> else <expression>
                                if <expression> then <expression>

<lvalue>                    ::= <identifier>
                                <postfix-expression>
                                
<assignment-expr>           ::= <lvalue> <- <expression>

<expression>                ::= <const-expr>
                                <assignment-expr>
                                <array-literal>
                                <if-expr>
                                { <expression-list> }

<const-expr>                ::= <logical-or-expression> ? <const-expr> : <const-expr>
                                <logical-or-expression>

<logical-or-expression>     ::= <logical-and-expression> || <logical-or-expression>
                                <logical-and-expression>
                            
<logical-and-expression>    ::= <inclusive-or-expression> && <logical-and-expression>
                                <inclusive-or-expression>

<inclusive-or-expression>   ::= <exclusive-or-expression> | <inclusive-or-expression>
                                <exclusive-or-expression>
                            
<exclusive-or-expression>   ::= <and-expression> ^ <exclusive-or-expression>
                                <and-expression>

<and-expression>            ::= <equity-expression> & <and-expression>
                                <equity-expression>

<equity-expression>         ::= <relational-expression> == <equity-expression>
                                <relational-expression> != <equity-expression>
                                <relational-expression>

<relational-expression>     ::= <shift-expression> >= <relational-expression>
                                <shift-expression> <= <relational-expression>
                                <shift-expression> > <relational-expression>
                                <shift-expression> < <relational-expression>
                                <shift-expression>

<shift-expression>          ::= <additive-expression> >> <shift-expression>
                                <additive-expression> << <shift-expression>
                                <additive-expression>

<additive-expression>       ::= <multiplicative-expression> + <additive-expression>
                                <multiplicative-expression> - <additive-expression>
                                <multiplicative-expression>

<multiplicative-expression> ::= <cast-expression> * <multiplicative-expression>
                                <cast-expression> / <multiplicative-expression>
                                <cast-expression> % <multiplicative-expression>
                                <cast-expression>

<cast-expression>           ::= cast <cast-expression> as <type-name>
                                <unary-expression>

<unary-expression>          ::= sizeof <unary-expression>
                                + <cast-expression>
                                - <cast-expression>
                                ! <cast-expression>
                                ~ <cast-expression>
                                <postfix-expression>

<argument-list>             ::= <expression> , <argument-list>
                                <expression>

<postfix-expression>        ::= <postfix-expression> [ <expression> ]
                                <postfix-expression> . <identifier>
                                <postfix-expression> ( <argument-list> )
                                <primary-expression>

<primary-expression>        ::= <identifier>
                                <literal>
                                ( <expression> )

<c-block-start>             ::= @{
<c-block-end>               ::= }@

*)

(*
Keywords:

    sizeof
    as
    if
    then
    else
    let
    rec
    mutable
    type
    delegate
    with
    set
    get
    and
    when
    match
    unsigned
    signed
    int
    char
    float
    double
    u64
    u32
    u16
    u8
    i64
    i32
    i16
    i8
    intptr
    uintptr

*)


//function pointers demonstration

type delegate myDelegate = int,int,int -> int
let discriminant (a:int,b:int,c:int -> int) = sqrt(b * b - 4 * a * c)
let funcptr:myDelegate = discriminant

//tuples as arugments
let addTuples (a:(int,int),b:(int,int) -> (int,int)) = (a[0] + b[0], a[1] + b[1])

(*
struct __t_2_3_3_intint {
    int _0;
    int _1;
};

__t_2_3_3_intint addTuples(__t_2_3_3_intint a, __t_2_3_3_intint b) {
    return ({
        ({
            __t_2_3_3_intint __TMP_TUPLE;
            __TMP_TUPLE._0 = a._0 + b._0;
            __TMP_TUPLE._1 = a._1 + b._1;
            __TMP_TUPLE;
        });
    });
}

*)

let swapfileDrive:char = 'C'
let swapfileSector:u64 = 98304                    //the 48 MB mark of the disk
let swapfileLength:int = 24576                    //12 MBs of swap file
let swapfileSectorsPerPage:int = 4096 / 512
let swapfileBitmap:u64* = null   

(*
char swapfileDrive = 'C';
uint64_t swapfileSector = 98304;
int swapfileLength = 24576;
int swapfileSectorsPerPage = 4096 / 512;
uint64_t* swapfileBitmap = nullptr;
*)

type VirtPageState (u8) =
    | Free = 0
    | Used
    | Start
    | End
    | StartAndEnd
    | Unusable = 0xF

(*

enum class __ut_VirtPageState : uint8_t {
    Free = 0,
    Used,
    Start,
    End,
    StartAndEnd,
    Unusable = 0xF
};

struct __u_VirtPageState {
    __ut_VirtPageState _type;
};

*)


let bitmap:u8* = VIRT_VIRT_PAGE_BITMAP

let manipulatePage (pageNum:u64 -> (u64,u64,u8*)) = (pageNum - VIRT_HEAP_MIN / 4096, (pageNum - VIRT_HEAP_MIN / 4096) & 1, bitmap + (pageNum - VIRT_HEAP_MIN / 4096) / 2)
//OR...
let manipulatePage (pageNum:u64 -> (u64,u64,u8*))  = {
    let p = pageNum - VIRT_HEAP_MIN / 4096
    (p, p & 1, bitmap + p / 2)
}

let setPageState (pageNum:u64 state:VirtPageState) = {
    let (_, nibble, ramAddr) = manipulatePage(pageNum)

    *ramAddr <- match nibble with 
                | 0 -> ( *ramAddr & 0xF0) | (state as u8)
                | 1 -> ( *ramAddr & 0x0F) | ((state as u8) << 4)
}

let swapIDToSector (id:u64) = id * swapfileSectorsPerPage + swapfileSector

let array1d:int[] = [20, 30, 40, 50]
let array2d:int[][] = [[1, 2, 3, 4],
                      [2, 3, 4, 5],
                      [3, 4, 5, 6]]
let test2:int[][2] = [[10, 100, 1000], 
                      [20, 200, 2000]]



type Shape = 
    | Circle {
        radius : int
    }
    | Square {
        length : int
    }
    | Rectangle {
        width : int
        height : int
    }

let area (s:Shape) = match s with
    Circle -> pi * s.radius * s.radius
    Square -> s.length * s.length
    Rectangle -> s.width * s.height


let a = Circle(radius = 50)
let b = Rectangle(width = 40, height = 60)
let aa = area(a)
let bb = area(b)

type Node = 
    | Tip                   //this is just an ENUM value...
    | Node {         
        left: Node
        right: Node
        val: int
    }

let rec sumTree (tree:Node) = match tree with 
    | Tip -> 0
    | Node -> tree.val + sumTree(tree.left) + sumTree(tree.right)



(*

enum class __ut_Node {
    Tip
    Node
};

struct __u_Node {
    __ut_Node _type;

    union {
        struct {
            __u_Node left;
            __u_Node right;
            int val;

        } __ui_Node;
    };
};

auto sumTree(__u_Node tree) {
    return ({
        if (tree._type == __ut_Node::Tip) {
            return 0;
        } else if (tree._type == __ut_Node::Node) {
            return tree.__ui_Node.val + sumTree(tree.__ui_Node.left) + sumTree(tree.__ui_Node.right);
        }
    });
}

*)


type BankAccount = {
    _name: string
    _balance: int

    let this = {
        _balance = 0
        _name = "Account Holder"
    }

    let this.inDebt ( -> bool) =  {
        _balance < 0
    }

    member this.balance with set(value) = _balance <- value
    member this.balance with get = balance

    //OR ...

    member this.name with set(value) = _name <- value
                     and get = _name
}


type public GDTEntry = {
    //all immutable after construction by default
    limit    : u64<0..15>
    base     : u64<0..23>
    accessed : u64<1>
    readWrite: u64<1>
    direction: u64<1>
    execute  : u64<1>
    type     : u64<1>
    priv     : u64<2>
    present  : u64<1>
    limit    : u64<16..19>
             : u64<1>
    bit64    : u64<4>
    size     : u64<1>
    gran     : u64<1>
    base     : u64<24..31>
    
    let this _base:u32 _limit:u32 = {
        base = _base
        limit = _limit
        
        accessed = 0
        readWrite = 0
        direction = 0
        execute = 0
        type = 0
        priv = 0
        present = 0
        bit64 = 0
        size = 0
        gran = 0
    }
}

(*
struct __s_GDTEntry {
    uint64_t __x_0_15_limit:16;
    uint64_t __x_0_23_base:24;
    uint64_t accessed:1;
    //etc..
    uint64_t __x_16_19_limit:4;
    //etc..
    uint64_t __x_24_31_base:8;

    __s_GDTEntry(uint32_t _base, uint32_t _limit) {
        __x_0_23_base  = (_base & 0b00000000111111111111111111111111) >> 0;
        __x_24_31_base = (_base & 0b11111111000000000000000000000000) >> 24;

        __x_0_15_limit  = (_limit & 0b00000000000000001111111111111111) >> 0;
        __x_16_19_limit = (_limit & 0b00000000000011110000000000000000) >> 16;

        accessed = 0;
        readWrite = 0;
        direction = 0;
        execute = 0;
        type = 0;
        priv = 0;
        present = 0;
        bit64 = 0;
        size = 0;
        gran = 0;
    }
}

*)

let mutable entries : GDTEntry[256]
let mutable numEntries : int = 0

let gdtAddEntry entry:GDTEntry = entries[numEntries++] <- entry

let gdtFlush = {
    gdtDescr.size = numEntries * 8 - 1
    gdtDescr.offset = entries
    loadGDT()
}

(*
GDTEntry entries[256];
int numEntries = 0;

auto gdtAddEntry(GDTEntry entry) {
    return ({
        entries[numEntries++] = entry;
    });
}

auto gdtFlush() {
    return ({
        gdtDescr.size = numEntries * 8 - 1;
        gdtDescr.offset = entries;
        loadGDT();
    });
}

*)

let gdtSetup = {
    let null = GDTEntry(0, 0)
    
    let code = GDTEntry(0, 0xFFFFF)
    
    let data = code with (execute = 1)
    let userCode = code with (priv = 3)
    let userData = data with (priv = 3)
    
    let code16 = code with (gran = 0, size = 0)
    let data16 = data with (gran = 0, size = 0)
    
    gdtAddEntry(null)
    gdtAddEntry(code)
    gdtAddEntry(data)
    gdtAddEntry(userCode)
    gdtAddEntry(userData)
    gdtAddEntry(code16)
    gdtAddEntry(data16)
    gdtFlush()
}

```
