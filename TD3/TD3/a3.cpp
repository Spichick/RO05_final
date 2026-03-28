#include <iostream>
#include <pthread.h>

struct Data {
    volatile bool stop;
    double counter;
    pthread_mutex_t mtx;
};

static void* incrementer(void* v_data) {
    Data* p_data = (Data*)v_data;

    while (not p_data->stop) {
        // section critique
        pthread_mutex_lock(&p_data->mtx);
        p_data->counter += 1.0;
        pthread_mutex_unlock(&p_data->mtx);
    }

    return v_data;
}

int main() {
    Data data{};
    data.stop = false;
    data.counter = 0.0;

    if (pthread_mutex_init(&data.mtx, nullptr) != 0) {
        std::cerr << "pthread_mutex_init failed\n";
        return 1;
    }

    pthread_t incrementThread[3];

    if (pthread_create(&incrementThread[0], nullptr, incrementer, &data) != 0 ||
        pthread_create(&incrementThread[1], nullptr, incrementer, &data) != 0 ||
        pthread_create(&incrementThread[2], nullptr, incrementer, &data) != 0) {
        std::cerr << "pthread_create failed\n";
        data.stop = true;
        pthread_mutex_destroy(&data.mtx);
        return 1;
    }

    std::cout << "Type 's' to stop: " << std::flush;
    for (char cmd = 'r'; cmd != 's' && (std::cin >> cmd); ) {
        std::cout << "Type 's' to stop: " << std::flush;
    }

    data.stop = true;

    for (int i = 0; i < 3; ++i) {
        pthread_join(incrementThread[i], nullptr);
    }

    pthread_mutex_lock(&data.mtx);
    double finalCounter = data.counter;
    pthread_mutex_unlock(&data.mtx);

    std::cout << "\nCounter value = " << finalCounter << std::endl;

    pthread_mutex_destroy(&data.mtx);
    return 0;
}