# meddle

## roadmap

Binary Expressions
- `=` Assign
- `+=` Add Assign
- `-=` Sub Assign
- `*=` Mult Assign
- `/=` Div Assign
- `%=` Mod Assign
- `&=` And Assign
- `|=` Or Assign
- `^=` Xor Assign
- `<<=` Left Shift Assign
- `>>=` Right Shift Assign

- `==` Equals
- `!=` Doesn't Equals
- `<` Less Than
- `<=` Less Than Equals
- `>` Greater Than
- `>=` Greater Than Equals

- `+` Add
- `-` Subtract
- `*` Multiply
- `/` Divide
- `%` Modulo
- `&` Bitwise And
- `|` Bitwise Or
- `^` Bitwise Xor
- `<<` Bitwise Left Shift
- `>>` Bitwise Right Shift
- `&&` Logical And
- `||` Logical Or

Unary Expressions
- `!` Logic Not (prefix)
- `~` Bitwise Not (prefix)
- `&` Address-of (prefix)
- `*` Dereference (prefix)
- `++` Increment (pre/post-fix)
- `--` Decrement (pre/post-fix)

- `CallExpr`
  - Aggregate Returns (aret)
  - Aggregate Arguments (aarg)

Enumerations
- `EnumDecl`

Structures
- `StructDecl`
- `StructInitExpr`
  - `FieldInitExpr`
- `MemberExpr`
- `MethodCallExpr`
- `TypeSpecExpr`

Array Initializers
- `SubscriptExpr`

Multi-package Support
- `UseDecl` declarations
- Declaration importing.
- Namespacing.
- `Segment` linkage.
- `PkgSpecExpr` (add uses to scope)

Inline Assembly

Templates
- `TemplateStructDecl`
- `TemplateFunctionDecl`
