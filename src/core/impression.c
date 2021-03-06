#include <assert.h>
#include <stdlib.h>
#include "impression.h"
#include "collections.h"

struct ImpressionInvoker {
    void *delegate_target;
    impression_invoker_delegate delegate;
    RoxList *handlers;
};

typedef struct ImpressionHandler {
    void *target;
    rox_impression_handler handler;
} ImpressionHandler;

ImpressionInvoker *impression_invoker_create() {
    ImpressionInvoker *invoker = calloc(1, sizeof(ImpressionInvoker));
    invoker->handlers = rox_list_create();
    return invoker;
}

ROX_INTERNAL void impression_invoker_set_delegate(
        ImpressionInvoker *impression_invoker,
        void *target,
        impression_invoker_delegate delegate) {
    impression_invoker->delegate_target = target;
    impression_invoker->delegate = delegate;
}

ROX_INTERNAL void impression_invoker_register(
        ImpressionInvoker *impression_invoker,
        void *target,
        rox_impression_handler handler) {
    assert(impression_invoker);
    assert(handler);
    ImpressionHandler *h = calloc(1, sizeof(ImpressionHandler));
    h->target = target;
    h->handler = handler;
    rox_list_add(impression_invoker->handlers, h);
}

ROX_INTERNAL void impression_invoker_invoke(
        ImpressionInvoker *impression_invoker,
        RoxReportingValue *value,
        ExperimentModel *experiment,
        RoxContext *context) {
    assert(impression_invoker);
    RoxExperiment *exp = experiment ? experiment_create(experiment) : NULL;
    if (impression_invoker->delegate) {
        impression_invoker->delegate(
                impression_invoker->delegate_target, value, exp, context);
    }
    ROX_LIST_FOREACH(h, impression_invoker->handlers, {
        ImpressionHandler *handler = (ImpressionHandler *) h;
        handler->handler(handler->target, value, context);
    })
    if (exp) {
        experiment_free(exp);
    }
}

ROX_INTERNAL void impression_invoker_free(ImpressionInvoker *impression_invoker) {
    assert(impression_invoker);
    rox_list_free_cb(impression_invoker->handlers, &free);
    free(impression_invoker);
}