#include <cbase.h>

#include "config.h"
#include "lang.h"
#include "parser.h"
#include "value.h"

bool expression_evaluator_init(ExpressionEvaluator *expression_evaluator) {
    list_init(&expression_evaluator->value_cache, sizeof(Value));
}

bool expression_evaluator_init_alloc(ExpressionEvaluator *expression_evaluator,
                                     size_t len) {
    return list_init_alloc(
        &expression_evaluator->value_cache,
        sizeof(Value),
        len
    );
}

bool expression_evaluator_evaluate(ExpressionEvaluator *expression_evaluator,
                                   Array *nodes,
                                   Value *context) {
}

void expression_evaluator_clear(ExpressionEvaluator *expression_evaluator) {
    list_clear(&expression_evaluator->value_cache);
}

void expression_evaluator_free(ExpressionEvaluator *expression_evaluator) {
    list_free(&expression_evaluator->value_cache);
}

/* vi: set et ts=4 sw=4: */
