#ifndef DATA_H__
#define DATA_H__

#define NUMBER1 "82349023489234902342323419041892349034189341.796"
#define NUMBER2 "13982309378334023462303455668934502028340345.289"
#define NUMBER3 "96331332867568925804626874710826851062529687.085"
#define NUMBER4      "023489234902342323419041892349034189341.796"
#define NUMBER5       "23489234902342323419041892349034189341.796"
#define NUMBER6       "23489234902342323419041892349034189341.79"

#define TEMPLATE \
"{{ include '/srv/http/templates/header.txt' }}\n"                         \
"\n"                                                                       \
"{{ if person.age >= 18 }}\n"                                              \
"...\n"                                                                    \
"{{ endif }}\n"                                                            \
"\n"                                                                       \
"{{ for person in people }}\n"                                             \
"    {{ person.name }} owes {{ person.income * tax_rate }} this year!\n"   \
"{{ endfor }}\n"                                                           \
"\n"                                                                       \
"{{ raw }}\n"                                                              \
"    This is how you would use raw }} and {{ markers.\n"                   \
"{{ endraw }}\n"                                                           \
"{{ if upper(message) == 'OR PUT THEM IN STRINGS {{ }} }} {{' }}\n"        \
"You guessed the magic message!\n"                                         \
"{{ endif }}\n"                                                            \
"\n"                                                                       \
"{{ for fib in [1, 1, 2, 3, 5, 8] }}\n"                                    \
"    Double {{ fib }}: {{ double(fib) }}\n"                                \
"{{ endfor }}\n"                                                           \
"{{ include '/srv/http/templates/footer.txt' }}\n"                         \
"Last little bit down here\n"

#define EXPRESSION_TEMPLATE "{{ 3 + 4 * 2 / (1 - 5) ^ 2 ^ 3 }}"
#define EXPRESSION_ANSWER "3.0001220703125"

#endif
