#ifndef TEMPLATE_H__
#define TEMPLATE_H__

enum {
    TEMPLATE_OPENING_FILE_FAILED = 1,
    TEMPLATE_SEEKING_IN_FILE_FAILED,
    TEMPLATE_READING_FILE_DATA_FAILED,
    TEMPLATE_FAILED_TO_REPLACE_INCLUDE_NODE,
};

typedef struct {
    Array nodes;
    ExpressionEvaluator expression_evaluator
} Template;

void template_init(Template *t);
bool template_init_alloc(Template *t, size_t node_cache_size,
                                      size_t expression_node_cache_size,
                                      Status *status);
bool template_parse_path(Template *t, const char *path, Status *status);
bool template_parse_data(Template *t, String *input, Status *status);
bool template_render_path(Template *t, const char *path, Value *context,
                                                         String *output,
                                                         Status *status);
bool template_render_data(Template *t, String *buf, String *output,
                                                    Status *status);
void template_clear(Template *t);
void template_free(Template *t);

#endif

/* vi: set et ts=4 sw=4: */
