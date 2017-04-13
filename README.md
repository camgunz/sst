# SST

SST is a simple template processor.  This is what it looks like:

    {{ include '/srv/http/templates/header.txt' }}

    {{ if person.age >= 18 }}
        Don't forget to register to vote!
    {{ else if person.age >= 14 }}
        If you'll be 18 before election day, register to vote!
    {{ endif }}

    {{ for person in people }}
        {{ person.name }} owes {{ person.income * tax_rate }} this year!
    {{ endfor }}

    {{ raw }}
        This is how you would use raw }} and {{ markers.
    {{ endraw }}

    {{ if upper(message) == 'OR PUT THEM IN STRINGS {{ }} }} {{' }}
        You guessed the magic message!
    {{ endif }}

    {{ for fib in [1, 1, 2, 3, 4, 5, 4 + 4, 13, 21, 34] }}
        {{ if fib == 4 }}
            {{ continue }}
        {{ endif }}
        Double {{ fib }}: {{ double(fib) }}
        (also {{ fibs[min(8, max(0, fib - 8))] }})
        {{ if fib >= 10 }}
            Whew fibs got too big: {{ fib }}
            {{ break }}
        {{ endif }}
    {{ endfor }}

    {{ include '/srv/http/templates/footer.txt' }}

    Last little bit down here

SST is still in heavy development, but a few things differentiate SST from
other template engines:

- Very simple
- Uses arbitrary-precision arithmetic behind the scenes
- Written in C (C99)
  - High throughput
  - Low latency
  - Low memory usage
- Does not use regular expressions

SST's parser is thread-aware: multiple parsers may be assigned on a 1:1 basis
to multiple threads.

SST's templates are thread-safe: templates may be rendered by multiple threads
simultaneously.

SST deals strictly in UTF-8.

## A template engine in C?  Are you out of your mind?

Maybe.  Here's the plan to avoid disaster:

1. Use [cbase](https://github.com/camgunz/cbase), a small C data structures
   library with test coverage approaching 90% (what's left is error
   conditions)
2. Create a comprehensive test suite for SST itself
3. Fuzz SST with AFL
4. Use strict compiler flags
5. Use sanitizers during development

SST is written in what I would consider clear, readable, and safe C.  It is
also not very big: currently the line count is ~3,800 LOC and will probably
stay under 5,000 LOC when fully implemented.  It should be easy to understand
and thus easy to audit.

## Why not use template engine X?

I wanted an engine I could:

- Use from the command-line or as a library
- Avoid fatal bugs (PHP, bad regex, string interpolation injection, etc.)
- Not wait for a big runtime (Java, C#) to start up
- Not use copious amounts of RAM (Python, Ruby, JS)
- Use arbitrary-precision arithmetic

Basically there are no templating languages in C, C++, Go or Rust that
use decimals and not regexes, so I wrote one.

## What does SST stand for?

NOAA tracks sea surface temperature (SST), and since this project is a
*C temp*lating engine...

