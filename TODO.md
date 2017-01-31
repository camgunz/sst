# To Do

Tokenizer: Basic tokens: text blocks, numbers, strings, symbols, keywords,
           words, and whitespace
Lexer: Code tokens: text blocks, numbers, strings, keywords,
                    identifier lookups, function lookups, index operations,
                    operators
Code block: an array of code tokens
Parser: Parses code tokens into an array of expressions, reducing them as far
        as they can go.
Template: Accepts input and passes it to its parser's array of expressions

Let's leave literals in their SSlice forms for now.  The parser will keep a
stack of Operands it uses as it parses expressions.

The parser will also keep an array of code tokens for each code block
