#include <roxx/client.h>
#include <cstdio>
#include "demo.hpp"

using namespace std;
using namespace Rox::Client;

int main(int argc, char **argv) {

    auto demoFlag = Flag::Create("demo.demoFlag", false);
    auto intFlag = Int::Create("demo.demoInt", 1, {1, 2, 3});
    auto doubleFlag = Double::Create("demo.demoDouble", 1.1, {1.1, 2.2, 3.3});
    auto stringFlag = String::Create("demo.demoStr", "red", {"red", "green", "blue"});

    Rox::Options *options = Rox::Client::OptionsBuilder()
            .SetDevModeKey(DEFAULT_DEV_MODE_KEY)
            .Build();

    Rox::Setup(DEFAULT_API_KEY, options);

    auto overrides = Overrides::Get();
    overrides->SetOverride(demoFlag->GetName(), "true");
    overrides->SetOverride(intFlag->GetName(), "2");
    overrides->SetOverride(doubleFlag->GetName(), "3.3");
    overrides->SetOverride(stringFlag->GetName(), "blue");

    bool loop = true;
    while (loop) {
        printf("Current flag values:\n");

        printf("Flag is %s%s\n",
               demoFlag->IsEnabled() ? "ON" : "OFF",
               overrides->HasOverride(demoFlag->GetName()) ? " (overridden)" : "");

        printf("String value is \"%s\"%s\n",
               stringFlag->GetValue(),
               overrides->HasOverride(stringFlag->GetName()) ? " (overridden)" : "");

        printf("Int value is %d%s\n",
               intFlag->GetValue(),
               overrides->HasOverride(intFlag->GetName()) ? " (overridden)" : "");

        printf("Double value is %f%s\n",
               doubleFlag->GetValue(),
               overrides->HasOverride(doubleFlag->GetName()) ? " (overridden)" : "");

        printf("\n");
        switch (prompt("1) Print values\n2) Clear overrides\n3) Exit")) {
            case '1':
                continue;
            case '2':
                overrides->Clear();
                continue;
            case '3':
                loop = false;
                break;
        }
    }

    Rox::Shutdown();
    return 0;
}
