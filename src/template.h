#ifndef TEMPLATE_H__
#define TEMPLATE_H__

typedef enum {
    TEMPLATE_OK = 0
} TemplateStatus;

Template*      template_from_path(const char *path);
Template*      template_from_data(const char *data);
TemplateStatus template_render(Template *template, Value *values);
char*          template_get_binary_ast(Template *template);

#endif

/* vi: set et ts=4 sw=4: */

