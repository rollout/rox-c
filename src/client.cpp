#include <cassert>
#include "roxx/client.h"

extern "C" {
#include "collections.h"
#include "util.h"
}

namespace Rox {

    namespace Client {

        ROX_API OptionsBuilder &OptionsBuilder::SetDefaultFreeze(::Rox::Client::Freeze freeze) {
            rox_options_set_default_freeze(_options, freeze);
            return *this;
        }

        ROX_API void Unfreeze(const char *ns) {
            if (ns == nullptr) {
                rox_unfreeze();
            } else {
                rox_unfreeze_ns(ns);
            }
        }

        ROX_API void String::Freeze(::Rox::Client::Freeze freeze) {
            rox_freeze_flag(_variant, freeze);
        }

        ROX_API void String::Unfreeze(::Rox::Client::Freeze freeze) {
            rox_unfreeze_flag(_variant, freeze);
        }

        ROX_API String *String::Create(const char *name, const char *defaultValue, ::Rox::Client::Freeze freeze) {
            assert(name);
            return new String(rox_add_string_with_freeze(name, defaultValue, freeze));
        }

        ROX_API String *String::Create(
                const char *name, const char *defaultValue, const std::vector<std::string> &options,
                ::Rox::Client::Freeze freeze) {
            assert(name);
            RoxList *list = ROX_EMPTY_LIST;
            for (auto &option : options) {
                rox_list_add(list, ROX_COPY(option.data()));
            }
            return new String(rox_add_string_with_freeze_and_options(name, defaultValue, list, freeze));
        }

        ROX_API void Flag::Freeze(::Rox::Client::Freeze freeze) {
            rox_freeze_flag(_variant, freeze);
        }

        ROX_API void Flag::Unfreeze(::Rox::Client::Freeze freeze) {
            rox_unfreeze_flag(_variant, freeze);
        }

        ROX_API Flag *Flag::Create(const char *name, bool defaultValue, ::Rox::Client::Freeze freeze) {
            return new Flag(rox_add_flag_with_freeze(name, defaultValue, freeze));
        }

        ROX_API void Int::Freeze(::Rox::Client::Freeze freeze) {
            rox_freeze_flag(_variant, freeze);
        }

        ROX_API void Int::Unfreeze(::Rox::Client::Freeze freeze) {
            rox_unfreeze_flag(_variant, freeze);
        }

        ROX_API Int *Int::Create(const char *name, int defaultValue, ::Rox::Client::Freeze freeze) {
            assert(name);
            return new Int(rox_add_int_with_freeze(name, defaultValue, freeze));
        }

        ROX_API Int *Int::Create(
                const char *name, int defaultValue, const std::vector<int> &options,
                ::Rox::Client::Freeze freeze) {
            assert(name);
            RoxList *list = ROX_EMPTY_LIST;
            for (auto it = options.begin(); it != options.end(); it++) {
                rox_list_add(list, mem_int_to_str(*it));
            }
            return new Int(rox_add_int_with_freeze_and_options(name, defaultValue, list, freeze));
        }

        ROX_API void Double::Freeze(::Rox::Client::Freeze freeze) {
            rox_freeze_flag(_variant, freeze);
        }

        ROX_API void Double::Unfreeze(::Rox::Client::Freeze freeze) {
            rox_unfreeze_flag(_variant, freeze);
        }

        ROX_API Double *Double::Create(const char *name, double defaultValue, ::Rox::Client::Freeze freeze) {
            assert(name);
            return new Double(rox_add_double_with_freeze(name, defaultValue, freeze));
        }

        ROX_API Double *Double::Create(const char *name, double defaultValue, const std::vector<double> &options,
                                       ::Rox::Client::Freeze freeze) {
            assert(name);
            RoxList *list = ROX_EMPTY_LIST;
            for (auto it = options.begin(); it != options.end(); it++) {
                rox_list_add(list, mem_double_to_str(*it));
            }
            return new Double(rox_add_double_with_freeze_and_options(name, defaultValue, list, freeze));
        }
    }
}