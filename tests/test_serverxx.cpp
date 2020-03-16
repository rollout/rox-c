#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <cassert>

#include "rollout.hpp"

extern "C" {
#include "core/impression/models.h"
#include "util.h"
}

class ServerTextContext {
private:
    Rox::CustomPropertyGeneratorInterface *AddGenerator(Rox::CustomPropertyGeneratorInterface *generator);

    Rox::ConfigurationFetchedHandlerInterface *_configurationFetchedHandler;
    Rox::ImpressionHandlerInterface *_impressionHandler;

public:
    ServerTextContext();

    virtual ~ServerTextContext();

    //
    // Container
    //

    Rox::Flag *simpleFlag;
    Rox::Flag *simpleFlagOverwritten;
    Rox::Flag *flagForImpression;
    Rox::Flag *flagForImpressionWithExperimentAndContext;
    Rox::Flag *flagCustomProperties;
    Rox::Flag *flagTargetGroupsAll;
    Rox::Flag *flagTargetGroupsAny;
    Rox::Flag *flagTargetGroupsNone;
    Rox::Variant *variantWithContext;
    Rox::Variant *variant;
    Rox::Variant *variantOverwritten;
    Rox::Flag *flagForDependency;
    Rox::Flag *flagDependent;
    Rox::Variant *flagColorDependentWithContext;

    //
    // TestVars
    //

    bool isImpressionRaised;
    bool isComputedBooleanPropCalled;
    bool isComputedStringPropCalled;
    bool isComputedIntPropCalled;
    bool isComputedDoublePropCalled;
    bool isComputedSemverPropCalled;
    bool targetGroup1;
    bool targetGroup2;
    bool isPropForTargetGroupForDependency;

    int configurationFetchedCount;

    char *lastImpressionValueName;
    char *lastImpressionValue;

    Rox::Experiment *lastImpressionExperiment;
    Rox::DynamicValue *lastImpressionContextValue;

    std::vector<Rox::CustomPropertyGeneratorInterface *> generators;
};

class TestConfigurationFetchedHandler : public Rox::ConfigurationFetchedHandlerInterface {
    ServerTextContext *_ctx;

public:
    explicit TestConfigurationFetchedHandler(ServerTextContext *ctx) : _ctx(ctx) {}

public:
    void ConfigurationFetched(Rox::ConfigurationFetchedArgs *args) override {
        if (args->fetcher_status == AppliedFromNetwork) {
            ++_ctx->configurationFetchedCount;
        }
    }
};

class TestImpressionHandler : public Rox::ImpressionHandlerInterface {
    ServerTextContext *_ctx;
public:
    explicit TestImpressionHandler(ServerTextContext *ctx) : _ctx(ctx) {}

    void HandleImpression(Rox::ReportingValue *value, Rox::Experiment *experiment, Rox::Context *context) override {
        if (!value) {
            return;
        }

        if (str_equals(value->name, "flagForImpression")) {
            _ctx->isImpressionRaised = true;
        }

        if (_ctx->lastImpressionValue) {
            free(_ctx->lastImpressionValue);
        }

        if (_ctx->lastImpressionValueName) {
            free(_ctx->lastImpressionValueName);
        }

        if (_ctx->lastImpressionExperiment) {
            experiment_free(_ctx->lastImpressionExperiment);
        }

        if (_ctx->lastImpressionContextValue) {
            rox_dynamic_value_free(_ctx->lastImpressionContextValue);
        }

        _ctx->lastImpressionValueName = mem_copy_str(value->name);
        _ctx->lastImpressionValue = mem_copy_str(value->value);
        _ctx->lastImpressionExperiment = experiment ? experiment_copy(experiment) : nullptr;
        if (context) {
            RoxDynamicValue *val = rox_context_get(context, "var");
            _ctx->lastImpressionContextValue = val ? rox_dynamic_value_create_copy(val) : nullptr;
        } else {
            _ctx->lastImpressionContextValue = nullptr;
        }
    }
};

class TestComputedStringProperty : public Rox::CustomPropertyGeneratorInterface {
    ServerTextContext *_ctx;
public:
    explicit TestComputedStringProperty(ServerTextContext *ctx) : _ctx(ctx) {}

    Rox::DynamicValue *operator()(Rox::Context *context) override {
        _ctx->isComputedStringPropCalled = true;
        return rox_dynamic_value_create_string_copy("World");
    }
};

class TestComputedBooleanProperty : public Rox::CustomPropertyGeneratorInterface {
    ServerTextContext *_ctx;
public:
    explicit TestComputedBooleanProperty(ServerTextContext *ctx) : _ctx(ctx) {}

    Rox::DynamicValue *operator()(Rox::Context *context) override {
        _ctx->isComputedBooleanPropCalled = true;
        return rox_dynamic_value_create_boolean(false);
    }
};

class TestComputedDoubleProperty : public Rox::CustomPropertyGeneratorInterface {
    ServerTextContext *_ctx;
public:
    explicit TestComputedDoubleProperty(ServerTextContext *ctx) : _ctx(ctx) {}

    Rox::DynamicValue *operator()(Rox::Context *context) override {
        _ctx->isComputedDoublePropCalled = true;
        return rox_dynamic_value_create_double(1.618);
    }
};

class TestComputedSemverProperty : public Rox::CustomPropertyGeneratorInterface {
    ServerTextContext *_ctx;
public:
    explicit TestComputedSemverProperty(ServerTextContext *ctx) : _ctx(ctx) {}

    Rox::DynamicValue *operator()(Rox::Context *context) override {
        _ctx->isComputedSemverPropCalled = true;
        return rox_dynamic_value_create_string_copy("20.7.1969");
    }
};

class TestComputedIntProperty : public Rox::CustomPropertyGeneratorInterface {
    ServerTextContext *_ctx;
public:
    explicit TestComputedIntProperty(ServerTextContext *ctx) : _ctx(ctx) {}

    Rox::DynamicValue *operator()(Rox::Context *context) override {
        _ctx->isComputedIntPropCalled = true;
        return rox_dynamic_value_create_int(28);
    }
};

struct TestComputedComputedPropertyUsingContext : public Rox::CustomPropertyGeneratorInterface {
    const char *_key;
public:
    explicit TestComputedComputedPropertyUsingContext(const char *key) : _key(key) {}

    Rox::DynamicValue *operator()(Rox::Context *context) override {
        RoxDynamicValue *value = rox_context_get(context, _key);
        return value ? rox_dynamic_value_create_copy(value) : nullptr;
    }
};

struct TestComputedComputedPropertyUsingValue : public Rox::CustomPropertyGeneratorInterface {
    bool *_value;
public:
    explicit TestComputedComputedPropertyUsingValue(bool *value) : _value(value) {}

    Rox::DynamicValue *operator()(Rox::Context *context) override {
        return rox_dynamic_value_create_boolean(*_value);
    }
};

static bool _CompareAndFree(char *str, const char *expected_value) {
    bool result = str && str_equals(str, expected_value);
    if (str) {
        free(str);
    }
    return result;
}

ServerTextContext::ServerTextContext() {
    Rox::Logging::SetLogLevel(RoxLogLevelDebug);

    _configurationFetchedHandler = new TestConfigurationFetchedHandler(this);
    _impressionHandler = new TestImpressionHandler(this);

    Rox::Options *options = Rox::OptionsBuilder()
            .SetConfigurationFetchedHandler(_configurationFetchedHandler)
            .SetImpressionHandler(_impressionHandler)
            .SetDevModeKey("f3be3b47c02bca33ae618130")
            .Build();

    this->simpleFlag = Rox::Flag::Create("simpleFlag", true);
    this->simpleFlagOverwritten = Rox::Flag::Create("simpleFlagOverwritten", true);
    this->flagForImpression = Rox::Flag::Create("flagForImpression", false);
    this->flagForImpressionWithExperimentAndContext = Rox::Flag::Create("flagForImpressionWithExperimentAndContext",
                                                                        false);
    this->flagCustomProperties = Rox::Flag::Create("flagCustomProperties", false);
    this->flagTargetGroupsAll = Rox::Flag::Create("flagTargetGroupsAll", false);
    this->flagTargetGroupsAny = Rox::Flag::Create("flagTargetGroupsAny", false);
    this->flagTargetGroupsNone = Rox::Flag::Create("flagTargetGroupsNone", false);
    this->variantWithContext = Rox::Variant::Create("variantWithContext", "red",
                                                    std::vector<std::string>{"red", "blue", "green"});
    this->variant = Rox::Variant::Create("variant", "red", std::vector<std::string>{"red", "blue", "green"});
    this->variantOverwritten = Rox::Variant::Create("variantOverwritten", "red",
                                                    std::vector<std::string>{"red", "blue", "green"});
    this->flagForDependency = Rox::Flag::Create("flagForDependency", false);
    this->flagDependent = Rox::Flag::Create("flagDependent", false);
    this->flagColorDependentWithContext = Rox::Variant::Create("flagColorDependentWithContext", "White",
                                                               std::vector<std::string>{"White", "Blue", "Green",
                                                                                        "Yellow"});

    this->isImpressionRaised = false;
    this->isComputedBooleanPropCalled = false;
    this->isComputedStringPropCalled = false;
    this->isComputedIntPropCalled = false;
    this->isComputedDoublePropCalled = false;
    this->isComputedSemverPropCalled = false;
    this->targetGroup1 = false;
    this->targetGroup2 = false;
    this->isPropForTargetGroupForDependency = false;
    this->configurationFetchedCount = 0;
    this->lastImpressionValueName = nullptr;
    this->lastImpressionValue = nullptr;
    this->lastImpressionExperiment = nullptr;
    this->lastImpressionContextValue = nullptr;
    this->generators = std::vector<Rox::CustomPropertyGeneratorInterface *>();

    Rox::SetCustomProperty<const char *>("stringProp1", "Hello");
    Rox::SetCustomComputedProperty<const char *>("stringProp2", AddGenerator(new TestComputedStringProperty(this)));

    Rox::SetCustomProperty<bool>("boolProp1", true);
    Rox::SetCustomComputedProperty<bool>("boolProp2", AddGenerator(new TestComputedBooleanProperty(this)));

    Rox::SetCustomProperty<int>("intProp1", 6);
    Rox::SetCustomComputedProperty<int>("intProp2", AddGenerator(new TestComputedIntProperty(this)));

    Rox::SetCustomProperty<double>("doubleProp1", 3.14);
    Rox::SetCustomComputedProperty<double>("doubleProp2", AddGenerator(new TestComputedDoubleProperty(this)));

    Rox::SetCustomSemverProperty("smvrProp1", "9.11.2001");
    Rox::SetCustomComputedSemverProperty("smvrProp2", AddGenerator(new TestComputedSemverProperty(this)));

    Rox::SetCustomComputedProperty<bool>(
            "boolPropTargetGroupForVariant",
            AddGenerator(new TestComputedComputedPropertyUsingContext("isDuckAndCover")));

    Rox::SetCustomComputedProperty<bool>(
            "boolPropTargetGroupForVariantDependency",
            AddGenerator(new TestComputedComputedPropertyUsingContext("isDuckAndCover")));

    Rox::SetCustomComputedProperty<bool>(
            "boolPropTargetGroupOperand1",
            AddGenerator(new TestComputedComputedPropertyUsingValue(&this->targetGroup1)));

    Rox::SetCustomComputedProperty<bool>(
            "boolPropTargetGroupOperand2",
            AddGenerator(new TestComputedComputedPropertyUsingValue(&this->targetGroup2)));

    Rox::SetCustomComputedProperty<bool>(
            "boolPropTargetGroupForDependency",
            AddGenerator(new TestComputedComputedPropertyUsingValue(&this->isPropForTargetGroupForDependency)));

    Rox::Setup("5e6f80212e4fee6222cc9d6c", options);
}

ServerTextContext::~ServerTextContext() {
    Rox::Shutdown();

    delete _configurationFetchedHandler;
    delete _impressionHandler;

    if (this->lastImpressionValue) {
        free(this->lastImpressionValue);
    }
    if (this->lastImpressionValueName) {
        free(this->lastImpressionValueName);
    }
    if (this->lastImpressionExperiment) {
        experiment_free(this->lastImpressionExperiment);
    }
    if (this->lastImpressionContextValue) {
        rox_dynamic_value_free(this->lastImpressionContextValue);
    }
    for (auto generator : this->generators) {
        delete generator;
    }
}

Rox::CustomPropertyGeneratorInterface *
ServerTextContext::AddGenerator(Rox::CustomPropertyGeneratorInterface *generator) {
    this->generators.push_back(generator);
    return generator;
}

TEST_CASE ("test_simple_flag", "[server]") {
    auto *ctx = new ServerTextContext();
    REQUIRE(ctx->simpleFlag->IsEnabled());
    delete ctx;
}

TEST_CASE ("test_simple_flag_overwritten", "[server]") {
    auto *ctx = new ServerTextContext();
    REQUIRE(!ctx->simpleFlagOverwritten->IsEnabled());
    delete ctx;
}

TEST_CASE ("test_variant", "[server]") {
    auto *ctx = new ServerTextContext();
    REQUIRE(_CompareAndFree(ctx->variant->GetValue(), "red"));
    delete ctx;
}

TEST_CASE ("test_variant_overwritten", "[server]") {
    auto *ctx = new ServerTextContext();
    REQUIRE(_CompareAndFree(ctx->variantOverwritten->GetValue(), "green"));
    delete ctx;
}

TEST_CASE ("testing_all_custom_properties", "[server]") {
    auto *ctx = new ServerTextContext();
    REQUIRE(ctx->flagCustomProperties->IsEnabled());
    REQUIRE(ctx->isComputedBooleanPropCalled);
    REQUIRE(ctx->isComputedDoublePropCalled);
    REQUIRE(ctx->isComputedIntPropCalled);
    REQUIRE(ctx->isComputedSemverPropCalled);
    REQUIRE(ctx->isComputedStringPropCalled);
    delete ctx;
}

TEST_CASE ("testing_fetch_within_timeout", "[server]") {
    auto *ctx = new ServerTextContext();
    int numberOfConfigFetches = ctx->configurationFetchedCount;
    Rox::Fetch();
    REQUIRE(ctx->configurationFetchedCount > numberOfConfigFetches);
    delete ctx;
}

TEST_CASE ("testing_variant_with_context", "[server]") {
    auto *ctx = new ServerTextContext();
    Rox::Context *somePositiveContext = Rox::ContextBuilder()
            .AddBoolValue("isDuckAndCover", true)
            .Build();
    Rox::Context *someNegativeContext = Rox::ContextBuilder()
            .AddBoolValue("isDuckAndCover", false)
            .Build();
    REQUIRE(_CompareAndFree(ctx->variantWithContext->GetValue(), "red"));
    REQUIRE(_CompareAndFree(ctx->variantWithContext->GetValue(somePositiveContext),
                            "blue"));
    REQUIRE(_CompareAndFree(ctx->variantWithContext->GetValue(someNegativeContext),
                            "red"));
    rox_context_free(somePositiveContext);
    rox_context_free(someNegativeContext);
    delete ctx;
}

TEST_CASE ("testing_target_groups_all_any_none", "[server]") {
    auto *ctx = new ServerTextContext();

    ctx->targetGroup1 = ctx->targetGroup2 = true;
    REQUIRE(ctx->flagTargetGroupsAll->IsEnabled());
    REQUIRE(ctx->flagTargetGroupsAny->IsEnabled());
    REQUIRE(!ctx->flagTargetGroupsNone->IsEnabled());

    ctx->targetGroup1 = false;
    REQUIRE(!ctx->flagTargetGroupsAll->IsEnabled());
    REQUIRE(ctx->flagTargetGroupsAny->IsEnabled());
    REQUIRE(!ctx->flagTargetGroupsNone->IsEnabled());

    ctx->targetGroup2 = false;
    REQUIRE(!ctx->flagTargetGroupsAll->IsEnabled());
    REQUIRE(!ctx->flagTargetGroupsAny->IsEnabled());
    REQUIRE(ctx->flagTargetGroupsNone->IsEnabled());

    delete ctx;
}

TEST_CASE ("testing_impression_handler", "[server]") {
    auto *ctx = new ServerTextContext();

    ctx->flagForImpression->IsEnabled();
    REQUIRE(ctx->isImpressionRaised);
    ctx->isImpressionRaised = false;

    Rox::Context *context = Rox::ContextBuilder()
            .AddStringValue("var", "val")
            .Build();

    bool flagImpressionValue = ctx->flagForImpressionWithExperimentAndContext->IsEnabled(context);
    REQUIRE(ctx->lastImpressionValue != nullptr);
    REQUIRE(str_equals("true", ctx->lastImpressionValue));
    REQUIRE(flagImpressionValue);
    REQUIRE(str_equals("flagForImpressionWithExperimentAndContext", ctx->lastImpressionValueName));

    REQUIRE(ctx->lastImpressionExperiment != nullptr);
    REQUIRE(str_equals("5e6f8b274365819b98feaf06", ctx->lastImpressionExperiment->identifier));
    REQUIRE(str_equals("flag for impression with experiment and context", ctx->lastImpressionExperiment->name));

    REQUIRE(ctx->lastImpressionContextValue);
    REQUIRE(rox_dynamic_value_is_string(ctx->lastImpressionContextValue));
    REQUIRE(str_equals(rox_dynamic_value_get_string(ctx->lastImpressionContextValue), "val"));

    rox_context_free(context);

    delete ctx;
}

TEST_CASE ("testing_flag_dependency", "[server]") {
    auto *ctx = new ServerTextContext();

    ctx->isPropForTargetGroupForDependency = true;
    REQUIRE(ctx->flagForDependency->IsEnabled());
    REQUIRE(!ctx->flagDependent->IsEnabled());

    ctx->isPropForTargetGroupForDependency = false;
    REQUIRE(ctx->flagDependent->IsEnabled());
    REQUIRE(!ctx->flagForDependency->IsEnabled());

    delete ctx;
}

TEST_CASE ("testing_variant_dependency_with_context", "[server]") {
    auto *ctx = new ServerTextContext();

    Rox::Context *somePositiveContext = Rox::ContextBuilder()
            .AddBoolValue("isDuckAndCover", true)
            .Build();

    Rox::Context *someNegativeContext = Rox::ContextBuilder()
            .AddBoolValue("isDuckAndCover", false)
            .Build();

    REQUIRE(_CompareAndFree(
            ctx->flagColorDependentWithContext->GetValue(),
            "White"));

    REQUIRE(_CompareAndFree(
            ctx->flagColorDependentWithContext->GetValue(someNegativeContext),
            "White"));

    REQUIRE(_CompareAndFree(
            ctx->flagColorDependentWithContext->GetValue(somePositiveContext),
            "Yellow"));

    rox_context_free(somePositiveContext);
    rox_context_free(someNegativeContext);

    delete ctx;
}
