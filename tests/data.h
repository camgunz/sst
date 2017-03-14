#ifndef DATA_H__
#define DATA_H__

#define NUMBER1 "82349023489234902342323419041892349034189341.796"
#define NUMBER2 "13982309378334023462303455668934502028340345.289"
#define NUMBER3 "96331332867568925804626874710826851062529687.085"
#define NUMBER4      "023489234902342323419041892349034189341.796"
#define NUMBER5       "23489234902342323419041892349034189341.796"
#define NUMBER6       "23489234902342323419041892349034189341.79"

#define EXPR "{{ 3 + 4 * 2 / (1 - 5) ^ 2 ^ 3 }}"

#define TEMPLATE \
"{{ include '/srv/http/templates/header.txt' }}\n"                          \
"\n"                                                                        \
"{{ if person.age >= 18 }}\n"                                               \
"    Don't forget to register to vote!\n"                                   \
"{{ else if person.age >= 14 }}\n"                                          \
"    If you'll be 18 before election day, register to vote!\n"              \
"{{ endif }}\n"                                                             \
"\n"                                                                        \
"{{ for person in people }}\n"                                              \
"    {{ person.name }} owes {{ person.income * tax_rate }} this year!\n"    \
"{{ endfor }}\n"                                                            \
"\n"                                                                        \
"{{ raw }}\n"                                                               \
"    This is how you would use raw }} and {{ markers.\n"                    \
"{{ endraw }}\n"                                                            \
"\n"                                                                        \
"{{ if upper(message) == 'OR PUT THEM IN STRINGS {{ }} }} {{' }}\n"         \
"    You guessed the magic message!\n"                                      \
"{{ endif }}\n"                                                             \
"\n"                                                                        \
"{{ for fib in [1, 1, 2, 3, 4, 5, 4 + 4, 13, 21, 34] }}\n"                  \
"    {{ if fib == 4 }}\n"                                                   \
"        {{ continue }}\n"                                                  \
"    {{ endif }}\n"                                                         \
"    Double {{ fib }}: {{ double(fib) }}\n"                                 \
"    (also {{ fibs[min(8, max(0, fib - 8))] }})\n"                          \
"    {{ if fib >= 10 }}\n"                                                  \
"        Whew fibs got too big: {{ fib }}\n"                                \
"        {{ break }}\n"                                                     \
"    {{ endif }}\n"                                                         \
"{{ endfor }}\n"                                                            \
"\n"                                                                        \
"{{ include '/srv/http/templates/footer.txt' }}\n"                          \
"\n"                                                                        \
"Last little bit down here\n"

#define EXPRESSION_TEMPLATE "{{ 3 + 4 * 2 / (1 - 5) ^ 2 ^ 3 }}"
#define EXPRESSION_ANSWER "3.0001220703125"

#endif
