#include <iostream>
#include "timespec_utils.h"
#include "chrono.h"

static void print_ts(const char* name, const timespec& t) {
    timespec n = timespec_normalize(t);
    std::cout << name << " = {sec=" << n.tv_sec << ", nsec=" << n.tv_nsec
              << "}  (" << timespec_to_ms(n) << " ms)\n";
}

int main() {
    std::cout << "=== Test timespec utils ===\n";

    timespec a = timespec_from_ms(2700.0);    // 2.7s
    timespec b = timespec_from_ms(-2700.0);   // -2.7s

    print_ts("a(2700ms)", a);
    print_ts("b(-2700ms)", b);

    timespec c = a + b;
    print_ts("c=a+b", c);

    timespec d = a - b;
    print_ts("d=a-b", d);

    std::cout << "Comparisons: \n";
    std::cout << "a > b ? " << (a > b) << "\n";
    std::cout << "a == a ? " << (a == a) << "\n";

    std::cout << "\n=== Test wait (sleep 200ms) ===\n";
    Chrono t;
    timespec delay = timespec_from_ms(200.0);
    timespec_wait(delay);
    std::cout << "elapsed after wait: " << t.lap_ms() << " ms\n";

    std::cout << "\n=== Test Chrono ===\n";
    Chrono ch;
    timespec_wait(timespec_from_ms(120.0));
    std::cout << "lap active: " << ch.lap_ms() << " ms\n";
    auto dur = ch.stop();
    print_ts("stop duration", dur);
    std::cout << "lap inactive: " << ch.lap_ms() << " ms\n";

    return 0;
}
