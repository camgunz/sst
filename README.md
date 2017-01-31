# SST

SST is a simple template processor.  It handles the following tags:

    {{ include "/srv/http/templates/header.txt" }}

    {{ if person.age >= 18 }}
    ...
    {{ endif }}

    {{ for person in people }}
        {{ person.name }} owes {{ person.income * tax_rate }} this year!
        {{ if person.name == "Barack Obama" }}
            {{ break }}
        {{ endif }}
    {{ endfor }}

    {{ raw }}
        This is how you would use raw }} and {{ markers.
    {{ endraw }}

    {{ include "/srv/http/templates/footer.txt" }}

SST is still in heavy development, but a few things differentiate SST from
other template engines:

- Very simple
- Uses arbitrary-precision arithmetic behind the scenes
- Written in C (C99)
  - High throughput
  - Low latency
  - Low memory usage
- Does not use regular expressions

## A template engine in C?  Are you out of your mind?

Maybe.  Here's the plan to avoid disaster:

1. Use [cbase](https://github.com/camgunz/cbase), a small C data structures
   library with test coverage approaching 90% (what's left is error
   conditions)
2. Create a comprehensive test suite for SST itself
3. Run American Fuzzy Lop on SST
4. Use strict compiler flags
5. Use sanitizers during development

SST is written in what I would consider clear, readable, and safe C.  It is
also not very big: currently the line count is ~1,700 LOC and will probably
stay under 3,000 LOC when fully implemented.  It should be easy to understand
and thus easy to audit.

## Why not use template engine X?

I wanted an engine I could:

- Use from the command-line OR as a library
- Avoid fatal bugs (PHP, bad regex, string interpolation injection, etc.)
- Not wait for a big runtime (Java, C#) to start up
- Not use copious amounts of RAM (Python, Ruby, JS)
- Use arbitrary-precision arithmetic

Basically there are no templating languages in C, C++, Go or Rust that
don't use regexes and do use decimals, so I wrote one.

## What does SST stand for?

NOAA tracks sea surface temperature (SST), and since this project is a
*C temp*lating engine...

