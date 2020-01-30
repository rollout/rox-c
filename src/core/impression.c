#include <assert.h>
#include "impression.h"

struct ROX_INTERNAL ImpressionInvoker {
    List *handlers;
};

typedef struct ROX_INTERNAL ImpressionHandler {
    void *target;
    impression_handler handler;
} ImpressionHandler;

ImpressionInvoker *impression_invoker_create() {
    ImpressionInvoker *invoker = calloc(1, sizeof(ImpressionInvoker));
    list_new(&invoker->handlers);
    return invoker;
}

void ROX_INTERNAL impression_invoker_register(
        ImpressionInvoker *impression_invoker,
        void *target,
        impression_handler handler) {
    assert(impression_invoker);
    assert(handler);
    ImpressionHandler *h = calloc(1, sizeof(ImpressionHandler));
    h->target = target;
    h->handler = handler;
    list_add(impression_invoker->handlers, h);
}

void ROX_INTERNAL impression_invoker_invoke(
        ImpressionInvoker *impression_invoker,
        ReportingValue *value,
        ExperimentModel *experiment,
        Context *context) {
    assert(impression_invoker);
    assert(value);
    Experiment *exp = experiment ? experiment_create(experiment) : NULL;
    LIST_FOREACH(h, impression_invoker->handlers, {
        ImpressionHandler *handler = (ImpressionHandler *) h;
        handler->handler(handler->target, value, exp, context);
    })
    experiment_free(exp);
}

void ROX_INTERNAL impression_invoker_free(ImpressionInvoker *impression_invoker) {
    assert(impression_invoker);
    list_destroy_cb(impression_invoker->handlers, &free);
    free(impression_invoker);
}