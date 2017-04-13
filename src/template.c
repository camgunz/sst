#include <cbase.h>
#include <stdio.h>

#include "config.h"

#include "lang.h"
#include "value.h"
#include "tokenizer.h"
#include "lexer.h"
#include "expression_parser.h"
#include "parser.h"
#include "template.h"

/*
 * [TODO] Create a cache system to avoid re-parsing templates (`include`, etc.)
 */

#define BUF_SIZE 2048

#define opening_file_failed(status) status_failure( \
    status,                                         \
    "template",                                     \
    TEMPLATE_OPENING_FILE_FAILED,                   \
    "Opening file failed"                           \
)

#define seeking_in_file_failed(status) status_failure( \
    status,                                            \
    "template",                                        \
    TEMPLATE_SEEKING_IN_FILE_FAILED,                   \
    "Seeking in file failed"                           \
)

#define reading_file_data_failed(status) status_failure( \
    status,                                              \
    "template",                                          \
    TEMPLATE_READING_FILE_DATA_FAILED,                   \
    "Reading file data failed"                           \
)

#define failed_to_replace_include_node(status) status_failure( \
    status,                                                    \
    "template",                                                \
    TEMPLATE_FAILED_TO_REPLACE_INCLUDE_NODE,                   \
    "Failed to replace include node"                           \
)

static bool template_render(Template *template, Value *context,
                                                String *output,
                                                Status *status) {
    String buf;
    Array included_nodes;

    if (!string_init(&buf, "", status)) {
        return false;
    }

    if (!string_ensure_capacity(&buf, INIT_RENDER_BUF_ALLOC, status)) {
        return false;
    }

    if (!array_init_alloc(&included_nodes, sizeof(ASTNode), INIT_NODE_ALLOC,
                                                            status)) {
        string_free(&buf);
        return false;
    }

    for (size_t i = 0; i < nodes->len; i++) {
        ASTNode *node = array_index_fast(nodes, i);

        if (node->type != AST_NODE_INCLUDE) {
            continue;
        }

        array_clear(&included_nodes);

        if (!string_assign_slice(&buf, &node->as.include, status)) {
            string_free(&buf);
            array_free(&included_nodes);
            return false;
        }

        if (!template_parse_path(&included_nodes, buf.data, status)) {
            string_free(&buf);
            array_free(&included_nodes);
            return false;
        }

        if (!array_delete(nodes, i, status)) {
            string_free(&buf);
            array_free(&included_nodes);
            return false;
        }

        array_insert_array_fast(nodes, &included_nodes, i);
    }

    for (size_t i = 0; i < nodes->len; i++) {
        ASTNode *node = array_index_fast(nodes, i);

        switch(node->type) {
            case AST_NODE_TEXT:
                if (!string_append(output, node->as.text, status)) {
                    string_free(&buf);
                    array_free(&included_nodes);
                    return false;
                }
                break;
            case AST_NODE_INCLUDE:
                string_free(&buf);
                array_free(&included_nodes);
                return failed_to_replace_include_node(status);
            case AST_NODE_EXPRESSION:
                if (!expression_evaluator_evaluate(
                        &t->expression_evaluator,
                        &t->parser.expression_parser.output,
                        context,
                        status)) {
                    string_free(&buf);
                    array_free(&included_nodes);
                    return false;
                }

                string_free(&buf);
                array_free(&included_nodes);
                return value_to_string(
                    &t->parser.expression_parser.output,
                    output,
                    status
                );
            case AST_NODE_CONDITIONAL:
                if (!expression_evaluator_evaluate(
                        &t->expression_evaluator,
                        &t->parser.expression_parser.output,
                        context,
                        status)) {
                    string_free(&buf);
                    array_free(&included_nodes);
                    return false;
                }

                if (t->parser.expression_parser.output.type != VALUE_BOOLEAN) {
                    string_free(&buf);
                    array_free(&included_nodes);
                    return non_boolean_expression_in_conditional(status);
                }

                if (t->parser.expression_parser.output.as.boolean) {
                    // Execute until ELSE or END
                }
                else {
                    // Skip until ELSE or END
                }
                break;
            case AST_NODE_ITERATION:
                node_index = i;

                // Have to build an iterator
                // Save node index
                //   (incidentally, this has to be updated if `include` happens)
                // Evaluate iterator expression: array
                // Make index 0 -> array.len
                // Assign iterator identifier to array_index(0)
                // Execute until BREAK, CONTINUE, or END
                break;
            case AST_NODE_ELSE:
            case AST_NODE_CONDITIONAL_END:
            case AST_NODE_BREAK:
            case AST_NODE_CONTINUE:
            case AST_NODE_ITERATION_END:
            default:
                string_free(&buf);
                array_free(&included_nodes);
                return invalid_ast(status);
        }
    }

    return status_ok(status);

error:
    string_free(&buf);
    array_free(&included_nodes);
    return false;
}

void template_init(Template *t) {
    array_init(&t->nodes);
    expression_evaluator_init(&t->expression_evaluator);
}

bool template_init_alloc(Template *t, size_t node_cache_size,
                                      size_t expression_node_cache_size,
                                      Status *status) {
    if (!array_init_alloc(&t->nodes, node_cache_size, status)) {
        return false;
    }

    if (!expression_evaluator_init_alloc(&t->expression_evaluator,
                                         expression_node_cache_size,
                                         status)) {
        array_free(&t->nodes);
        return false;
    }

    return status_ok(status);
}

bool template_parse_path(Template *t, const char *path, Status *status) {
    char buf[BUF_SIZE];
    String s;
    FILE *template_file = fopen(path, "rb");
    size_t file_size = 0;

    if (!template_file) {
        return opening_file_failed(status);
    }

    if (fseek(template_file, 0, SEEK_END) == -1) {
        return seeking_in_file_failed(status);
    }

    file_size = ftell(template_file);

    if (fseek(template_file, 0, SEEK_SET) == -1) {
        return seeking_in_file_failed(status);
    }

    if (!string_init(&s, "", status)) {
        return false;
    }

    if (!string_ensure_capacity(&s, file_size + 1, status)) {
        return false;
    }

    while (true) {
        size_t bytes_read;

        bytes_read = fread(buf, sizeof(buf[0]), sizeof(buf), template_file);

        if (bytes_read > 0) {
            if (!string_append_cstr_len(&s, buf, bytes_read, status)) {
                fclose(template_file);
                string_free(&s);
                return false;
            }
        }

        if (bytes_read != (sizeof(buf[0]) * sizeof(buf))) {
            if (!feof(template_file)) {
                fclose(template_file);
                string_free(&s);
                return reading_file_data_failed(status);
            }

            break;
        }
    }

    fclose(template_file);

    if (!template_parse_data(t, &s, status)) {
        string_free(&s);
        return false;
    }

    string_free(&s);

    return status_ok(status);
}

bool template_parse_data(Template *t, String *input, Status *status) {
    Parser parser;
    SSlice data;

    if (!string_slice(buf, 0, buf->len, &data)) {
        return false;
    }

    if (!parser_init(&parser, &data, status)) {
        return false;
    }

    while (true) {
        ASTNode *node = NULL;
        
        if (!array_append(&t->nodes, (void **)&node, status)) {
            parser_free(&parser);
            return false;
        }

        if (!parser_load_next(&parser, node, status)) {
            if (!status_match(status, "parser", PARSER_EOF)) {
                parser_free(&parser);
                return false;
            }

            break;
        }
    }

    parser_free(&parser);

    return status_ok(status);
}

bool template_render_path(Template *t, const char *path, Value *context,
                                                         String *output,
                                                         Status *status) {
    return (
        template_parse_path(t, path, status) &&
        template_render(t, context, output, status)
    );
}

bool template_render_data(Template *t, String *input, Value *context,
                                                      String *output,
                                                      Status *status) {
    return (
        template_parse_data(t, input, status) &&
        template_render(t, context, output, status)
    );
}

void template_clear(Template *t) {
    array_clear(&t->nodes);
    expression_evaluator_clear(&t->expression_evaluator);
}

void template_free(Template *t) {
    array_free(&t->nodes);
    expression_evaluator_free(&t->expression_evaluator);
}
/* vi: set et ts=4 sw=4: */
