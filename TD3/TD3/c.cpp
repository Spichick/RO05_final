#include <iostream>
#include <unistd.h>
#include "Thread.h"
#include "Mutex.h"

struct Data {
    volatile bool stop;
    double counter;
    Mutex mtx;

    Data() : stop(false), counter(0.0), mtx(false) {}
};

class IncrementerThread : public Thread {
public:
    IncrementerThread(int id, Data& d) : Thread(id), data_(d) {}

protected:
    void run() override {
        while (!data_.stop) {
            Mutex::Lock lock(data_.mtx);
            data_.counter += 1.0;
        }
    }

private:
    Data& data_;
};

int main() {
    Thread::setMainSched(SCHED_OTHER);

    Data data;

    IncrementerThread t(1, data);

    t.start(0);

    std::cout << "Type 's' to stop: " << std::flush;
    for (char cmd = 'r'; cmd != 's' && (std::cin >> cmd); ) {
        std::cout << "Type 's' to stop: " << std::flush;
    }

    data.stop = true;

    t.join();

    {
        Mutex::Lock lock(data.mtx);
        std::cout << "\nCounter value = " << data.counter << std::endl;
    }

    std::cout << "t duration_ms = " << t.duration_ms() << " ms\n";

    return 0;
}