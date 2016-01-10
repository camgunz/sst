#ifndef PARSER_H__
#define PARSER_H__

typedef String TextBlock;

/*
 * {{ include "path/to/template.html" }}
 */
typedef struct {
    String literal;
    String path;
} Include;

/*
 * {{ site.name }}
 * {{ site.name | capitalize }}
 */
typedef struct {
    String expression;
    String filter_name; /* optional */
} Expression;

/*
 * {{ if site.name == "Super Site" }}
 * {{ if person.is_pilot }}
 * {{ elif !person.is_driver }}
 * {{ else }}
 * {{ endif }}
 */
typedef struct {
    String literal;
    String block;
    BoolOp operator;    /* optional */
    String expression1; /* optional */
    String expression2; /* optional */
} Conditional;

/*
 * {{ for fruit in fruits }}
 * {{ for name, age in people_to_ages }}
 * {{ endfor }}
 */
typedef struct {
    String literal;
    String block;
    String identifiers;
    String expression;
} Iteration;

/*
 * {{ raw }}
 * {{ endraw }}
 */
typedef struct {
    String literal;
    String block;
} Raw;

#endif

