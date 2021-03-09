#include <roxx/client.h>
#include <cstdio>
#include "demo.hpp"

using namespace std;
using namespace Rox::Client;

int main(int argc, char **argv) {

    auto demoFlag = Flag::Create("demo.demoFlag", false);
    auto intFlag = Int::Create("demo.demoInt", 1, {1, 2, 3});
    auto doubleFlag = Double::Create("demo.demoDouble", 1.1, {1.1, 2.2, 3.3});
    auto stringFlag = String::Create("demo.demoStr", "red", {"red", "green", "blue"},
                                     RoxFreezeNone);

    Rox::Options *options = Rox::Client::OptionsBuilder()
            .SetDefaultFreeze(RoxFreezeUntilLaunch)
            .SetDevModeKey(DEFAULT_DEV_MODE_KEY)
            .Build();

    Rox::Setup(DEFAULT_API_KEY, options);

    bool loop = true;
    while (loop) {
        printf("Current flag values:\n");
        printf("Flag is %s\n", demoFlag->IsEnabled() ? "ON" : "OFF");
        printf("String value is \"%s\"\n", stringFlag->GetValue());
        printf("Int value is %d\n", intFlag->GetValue());
        printf("Double value is %f\n", doubleFlag->GetValue());
        printf("\n");
        switch (prompt("1) Unfreeze double\n2) Unfreeze bool\n3) Unfreeze int\n4) Unfreeze all\n5) Print\n6) Exit")) {
            case '1':
                doubleFlag->Unfreeze();
                break;
            case '2':
                demoFlag->Unfreeze();
                break;
            case '3':
                intFlag->Unfreeze();
                break;
            case '4':
                Unfreeze();
                continue;
            case '5':
                continue;
            case '6':
                loop = false;
                break;
        }
    }

    Rox::Shutdown();
    return 0;
}
