#include <assert.h>
#include "impression.h"

struct ROX_INTERNAL ImpressionInvoker {
    impression_handler handler;
};

ImpressionInvoker *impression_invoker_create() {
    return calloc(1, sizeof(ImpressionInvoker));
}

void ROX_INTERNAL impression_invoker_register(
        ImpressionInvoker *impression_invoker,
        impression_handler handler) {
    assert(impression_invoker);
    assert(handler);
    impression_invoker->handler = handler;
}

void ROX_INTERNAL impression_invoker_invoke(
        ImpressionInvoker *impression_invoker,
        ReportingValue *value,
        ExperimentModel *experiment,
        Context *context) {
    assert(impression_invoker);
    assert(value);
    if (impression_invoker->handler) {
        impression_invoker->handler(value, experiment_create(experiment), context);
    }
}

void ROX_INTERNAL impression_invoker_free(ImpressionInvoker *impression_invoker) {
    assert(impression_invoker);
    free(impression_invoker);
}