## C/F# Compiler

A compiler for a custom language combining C, F# and some other things.


Example:
```
let mutable a: int = 6
a <- !a + 9
let b: int = 2 + a * 88

let mutable variable: int = 123
variable <- 123 * (2 + 55)

let test: int = !(variable > 444)
let test2: int = !!(variable > 666)

let c = (a + 5) || b

let e = (a + 4) ? (b + 5) : (c + 6)
let f = a ?? b


```
