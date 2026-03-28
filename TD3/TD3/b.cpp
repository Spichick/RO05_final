#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "Mutex.h"

struct Data {
    volatile bool stop;
    double counter;
    Mutex mtx;

    Data() : stop(false), counter(0.0), mtx(false) {}
};

static void* incrementer(void* v_data) {
    auto* d = static_cast<Data*>(v_data);

    while (!d->stop) {
        Mutex::Lock lock(d->mtx);
        d->counter += 1.0;
    }
    return nullptr;
}

int main() {
    Data data;

    pthread_t th[3];
    for (int i = 0; i < 3; ++i) {
        if (pthread_create(&th[i], nullptr, incrementer, &data) != 0) {
            std::cerr << "pthread_create failed\n";
            data.stop = true;
            for (int j = 0; j < i; ++j) pthread_join(th[j], nullptr);
            return 1;
        }
    }

    std::cout << "Type 's' to stop: " << std::flush;
    for (char cmd = 'r'; cmd != 's' && (std::cin >> cmd); ) {
        std::cout << "Type 's' to stop: " << std::flush;
    }
    data.stop = true;

    for (auto& t : th) pthread_join(t, nullptr);

    {
        Mutex::Lock lock(data.mtx);
        std::cout << "\nCounter value = " << data.counter << std::endl;
    }
    return 0;
}