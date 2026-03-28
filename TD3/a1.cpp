#include <iostream>
#include <pthread.h>

static void* incrementer(void* v_stop) {
    volatile bool* p_stop = (volatile bool*)v_stop;
    double counter = 0.0;

    while (not *p_stop) {
        counter += 1.0;
    }

    std::cout << "\nCounter value = " << counter << std::endl;
    return v_stop;
}

int main() {
    volatile bool stop = false;
    pthread_t incrementThread;

    if (pthread_create(&incrementThread, nullptr, incrementer, (void*)&stop) != 0) {
        std::cerr << "pthread_create failed\n";
        return 1;
    }

    std::cout << "Type 's' to stop: " << std::flush;
    for (char cmd = 'r'; cmd != 's' && (std::cin >> cmd); ) {
        std::cout << "Type 's' to stop: " << std::flush;
    }

    stop = true;

    pthread_join(incrementThread, nullptr);
    return 0;
}