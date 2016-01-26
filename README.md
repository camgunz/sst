# SST

SST is a simple template processor.  It handles the following tags:

    {{ include "/srv/http/templates/header.txt" }}

    {{ if person.age >= 18 }}
    ...
    {{ endif }}

    {{ for person in people }}
        {{ person.name }} owes {{ person.income * tax_rate }} this year!
    {{ endfor }}

    {{ raw }}
        This is how you would use raw }} and {{ markers.
    {{ endraw }}

    {{ include "/srv/http/templates/footer.txt" }}

SST is still in heavy development, but a few things differentiate SST from
other template engines:

- Very simple
- Uses arbitrary-precision arithmetic behind the scenes
- Written in C
  - High performance
  - Low memory usage

## What does SST stand for?

NOAA tracks sea surface temperature (SST), and since this project is a
*C temp*lating engine...

