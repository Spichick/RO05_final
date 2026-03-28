#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <sched.h>
#include <unistd.h>

#include "Thread.h"
#include "Mutex.h"

class Calibrator {
public:
    Calibrator() { calibrate(); }

    long long loopsPerMs() const { return loops_per_ms_; }

private:
    long long loops_per_ms_{100000};
    static inline void burn(long long n) {
        volatile long long x = 0;
        for (long long i = 0; i < n; ++i) x += i;
        (void)x;
    }

    static timespec now() {
        timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts;
    }

    static long diff_ms(const timespec& a, const timespec& b) {
        long sec  = static_cast<long>(b.tv_sec - a.tv_sec);
        long nsec = static_cast<long>(b.tv_nsec - a.tv_nsec);
        if (nsec < 0) { sec -= 1; nsec += 1000000000L; }
        return sec * 1000L + nsec / 1000000L;
    }

    void calibrate() {
        long long test_loops = 2000000LL;
        timespec t0 = now();
        burn(test_loops);
        timespec t1 = now();

        long ms = diff_ms(t0, t1);
        if (ms <= 0) ms = 1;
        loops_per_ms_ = test_loops / ms;
        if (loops_per_ms_ <= 0) loops_per_ms_ = 100000;
    }
};

class CpuLoop {
public:
    explicit CpuLoop(const Calibrator& c) : cal_(c) {}

    void run_ms(long ms) {
        if (ms < 0) ms = 0;
        const long long loops = static_cast<long long>(ms) * cal_.loopsPerMs();
        burn(loops);
    }

private:
    const Calibrator& cal_;

    static inline void burn(long long n) {
        volatile long long x = 1;
        for (long long i = 0; i < n; ++i) x = x * 1664525 + 1013904223;
        (void)x;
    }
};

static timespec now() {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

static long diff_ms(const timespec& a, const timespec& b) {
    long sec  = static_cast<long>(b.tv_sec - a.tv_sec);
    long nsec = static_cast<long>(b.tv_nsec - a.tv_nsec);
    if (nsec < 0) { sec -= 1; nsec += 1000000000L; }
    return sec * 1000L + nsec / 1000000L;
}

static void sleep_ms(long ms) {
    if (ms <= 0) return;
    usleep(static_cast<useconds_t>(ms * 1000));
}

struct Report {
    timespec tStart{};
    timespec tEnd{};
    bool hasStart{false};
    bool hasEnd{false};
};

class Thread_C : public Thread {
public:
    Thread_C(int id, Mutex& resource, const Calibrator& cal, Report& rep, timespec t0)
        : Thread(id), resource_(resource), cpu_(cal), rep_(rep), t0_(t0) {}

protected:
    void run() override {
        rep_.tStart = now(); rep_.hasStart = true;

        Mutex::Lock lock(resource_);
        cpu_.run_ms(300);

        rep_.tEnd = now(); rep_.hasEnd = true;
    }

private:
    Mutex& resource_;
    CpuLoop cpu_;
    Report& rep_;
    timespec t0_;
};

class Thread_A : public Thread {
public:
    Thread_A(int id, Mutex& resource, const Calibrator& cal, Report& rep, long startDelayMs)
        : Thread(id), resource_(resource), cpu_(cal), rep_(rep), delay_(startDelayMs) {}

protected:
    void run() override {
        sleep_ms(delay_);
        rep_.tStart = now(); rep_.hasStart = true;

        Mutex::Lock lock(resource_);
        cpu_.run_ms(50);

        rep_.tEnd = now(); rep_.hasEnd = true;
    }

private:
    Mutex& resource_;
    CpuLoop cpu_;
    Report& rep_;
    long delay_;
};

class Thread_B : public Thread {
public:
    Thread_B(int id, const Calibrator& cal, Report& rep, long startDelayMs)
        : Thread(id), cpu_(cal), rep_(rep), delay_(startDelayMs) {}

protected:
    void run() override {
        sleep_ms(delay_);
        rep_.tStart = now(); rep_.hasStart = true;

        cpu_.run_ms(400);

        rep_.tEnd = now(); rep_.hasEnd = true;
    }

private:
    CpuLoop cpu_;
    Report& rep_;
    long delay_;
};

static void print_report(const std::string& name, const Report& r, const timespec& tC0) {
    const long start_ms = r.hasStart ? diff_ms(tC0, r.tStart) : -1;
    const long end_ms   = r.hasEnd   ? diff_ms(tC0, r.tEnd)   : -1;
    const long dur_ms   = (r.hasStart && r.hasEnd) ? diff_ms(r.tStart, r.tEnd) : -1;

    std::cout << "  " << name
              << " start=" << start_ms << "ms"
              << " end=" << end_ms << "ms"
              << " duration=" << dur_ms << "ms\n";
}

static void run_pass(bool inversionSafe, int policy) {
    std::cout << "\n=== PASS: inversionSafe=" << (inversionSafe ? "true" : "false") << " ===\n";

    Thread::setMainSched(policy);

    Calibrator calibrator;

    Mutex resource(inversionSafe);

    Report repA, repB, repC;

    timespec tC0{};

    const int pmin = Thread::getMinPrio(policy);
    const int pmax = Thread::getMaxPrio(policy);

    int prioA = (policy == SCHED_OTHER) ? 0 : (pmax);
    int prioB = (policy == SCHED_OTHER) ? 0 : (pmin + (pmax - pmin) / 2);
    int prioC = (policy == SCHED_OTHER) ? 0 : (pmin);

    Thread_C tC(1, resource, calibrator, repC, now());
    Thread_A tA(2, resource, calibrator, repA, 10);
    Thread_B tB(3, calibrator, repB, 20);

    tC.start(prioC);
    while (!repC.hasStart) { /* spin briefly */ }
    tC0 = repC.tStart;

    tA.start(prioA);
    tB.start(prioB);

    tA.join();
    tB.join();
    tC.join();

    std::cout << "Results relative to task_C start:\n";
    print_report("task_C (low)", repC, tC0);
    print_report("task_A (high)", repA, tC0);
    print_report("task_B (mid)", repB, tC0);

    std::cout << "Thread durations (run() only):\n";
    std::cout << "  A duration_ms()=" << tA.duration_ms() << "ms\n";
    std::cout << "  B duration_ms()=" << tB.duration_ms() << "ms\n";
    std::cout << "  C duration_ms()=" << tC.duration_ms() << "ms\n";
}

int main(int argc, char** argv) {
    int policy = SCHED_FIFO;
    if (argc >= 2) {
        std::string p = argv[1];
        if (p == "SCHED_OTHER") policy = SCHED_OTHER;
        else if (p == "SCHED_RR") policy = SCHED_RR;
        else if (p == "SCHED_FIFO") policy = SCHED_FIFO;
        else {
            std::cerr << "Usage: " << argv[0] << " [SCHED_OTHER|SCHED_RR|SCHED_FIFO]\n";
            return 1;
        }
    }

    run_pass(false, policy);

    run_pass(true, policy);

    std::cout << "\nNote: Real-time policies (RR/FIFO) usually require root.\n";
    return 0;
}