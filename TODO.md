# To Do

- Really the only supported whitespace is a space, so move to that

- Start parser
  - Parse code tokens into an AST
    - reducing expressions

- Start template:
  - Accepts data or a file name and passes it to a parser to get an AST
  - Accepts input and renders it through the AST
  - Will optionally use a cache for `include` statements

- TemplateCache:
  - Stores parsed templates so `include` blocks don't need to re-parse
    templates (would be nice to detect cycles at runtime too).
  - Some threading concerns however.

## Notes:

- leave literals in their SSlice forms for now.  The parser will keep a stack
  of `Operand`s it uses as it parses expressions.

- parser should keep a cache of code tokens
  - I think this means pulling the `CodeToken` out of `Lexer` and having its
    `*next` functions accept a `CodeToken` argument.
