# To Do

- Tokenizer:
  - Basic tokens:
    - text blocks
    - numbers
    - strings
    - symbols
    - keywords
    - words
    - whitespace
  - There's really no need for the extra RAW token type, just chop off the tags
    and count them as TEXT
  - There's no need for coalescing either, there can just be multiple TEXT
    nodes

- Lexer:
  - Code tokens:
    - text blocks
    - numbers
    - strings
    - keywords,
    - identifier lookups
    - function lookups
    - index operations
    - operators

- Parser:
  - parses code tokens into an AST
  - reducing expressions

- Template:
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
