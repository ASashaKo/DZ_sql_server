#include "utils.h"

auto print_os_data() -> void {
    struct utsname utsname;
    uname(&utsname);
    printf("OS name: %s (%s:%s)\n", utsname.sysname, utsname.release, utsname.version);
}




