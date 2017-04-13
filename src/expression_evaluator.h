#ifndef EXPRESSION_EVALUATOR_H__
#define EXPRESSION_EVALUATOR_H__

enum {
};

typedef struct {
    List value_cache;
    Value result;
} ExpressionEvaluator;

bool expression_evaluator_init(ExpressionEvaluator *expression_evaluator);
bool expression_evaluator_init_alloc(ExpressionEvaluator *expression_evaluator
                                     size_t len);
bool expression_evaluator_evaluate(ExpressionEvaluator *expression_evaluator,
                                   Array *nodes,
                                   Value *context);
void expression_evaluator_clear(ExpressionEvaluator *expression_evaluator);
void expression_evaluator_free(ExpressionEvaluator *expression_evaluator);

#endif

/* vi: set et ts=4 sw=4: */

