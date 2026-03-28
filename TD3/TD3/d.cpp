#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <sched.h>

#include "Thread.h"
#include "Counter.h"
#include "Incrementer.h"
#include "Chrono.h"

static long long parse_ll(const char* s, const char* name) {
    try {
        std::string str(s);
        size_t idx = 0;
        long long v = std::stoll(str, &idx);
        if (idx != str.size()) throw std::runtime_error("");
        return v;
    } catch (...) {
        throw std::runtime_error(std::string("Invalid ") + name + ": " + s);
    }
}

static int parse_policy(const std::string& s) {
    if (s == "SCHED_OTHER") return SCHED_OTHER;
    if (s == "SCHED_RR")    return SCHED_RR;
    if (s == "SCHED_FIFO")  return SCHED_FIFO;
    throw std::runtime_error("Unknown policy: " + s);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr
            << "Usage: " << argv[0]
            << " nLoops nTasks [protect] [SCHED_OTHER|SCHED_RR|SCHED_FIFO]\n";
        return 1;
    }

    const long long nLoops = parse_ll(argv[1], "nLoops");
    const long long nTasks = parse_ll(argv[2], "nTasks");
    bool protect = false;
    int policy = SCHED_OTHER;

    if (argc >= 4) {
        std::string arg3 = argv[3];
        if (arg3 == "protect") protect = true;
        else throw std::runtime_error("3rd argument must be 'protect'");
    }

    Thread::setMainSched(policy);

    Counter counter(protect);

    std::vector<std::unique_ptr<Incrementer>> tasks;
    tasks.reserve(static_cast<size_t>(nTasks));

    Chrono chrono;
    const timespec t0 = Chrono::now();

    for (int i = 0; i < nTasks; ++i) {
        auto th = std::make_unique<Incrementer>(i + 1, counter, static_cast<double>(nLoops));
        th->setMainT0(t0);
        tasks.push_back(std::move(th));
    }

    int prio = 0;
    if (policy != SCHED_OTHER) {
        prio = Thread::getMinPrio(policy);
    }

    for (auto& t : tasks) t->start(prio);
    for (auto& t : tasks) t->join();

    const double elapsed_s = chrono.seconds();

    const double value = counter.value();
    const double expected = static_cast<double>(nTasks) * static_cast<double>(nLoops);
    const double ratio = (expected > 0.0) ? (100.0 * value / expected) : 0.0;

    std::cout << "protect=" << (protect ? "yes" : "no")
              << ", policy=" << (policy == SCHED_OTHER ? "SCHED_OTHER" :
                                 policy == SCHED_RR    ? "SCHED_RR"    : "SCHED_FIFO")
              << "\n";

    std::cout << "Counter = " << std::fixed << std::setprecision(0) << value
              << " , expected = " << expected
              << " , ratio = " << std::setprecision(2) << ratio << "%\n";

    std::cout << "Total time = " << std::fixed << std::setprecision(1)
              << elapsed_s << " s\n";

    std::cout << "Per-task timing:\n";
    for (size_t i = 0; i < tasks.size(); ++i) {
        std::cout << "  T" << (i + 1)
                  << " startOffset=" << tasks[i]->startOffsetMs() << " ms"
                  << " duration=" << tasks[i]->duration_ms() << " ms\n";
    }

    return 0;
}