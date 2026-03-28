#include "Semaphore.h"

#include <pthread.h>
#include <iostream>
#include <vector>
#include <cstdlib>

struct ProducerData
{
    int id;
    Semaphore* sem;
    unsigned long tokensToProduce;
    unsigned long producedCount;
};

struct ConsumerData
{
    int id;
    Semaphore* sem;
    unsigned long consumedCount;
};

void* producer_run(void* arg)
{
    ProducerData* data = static_cast<ProducerData*>(arg);

    for (unsigned long i = 0; i < data->tokensToProduce; ++i)
    {
        data->sem->give();
        ++data->producedCount;
    }

    return nullptr;
}

void* consumer_run(void* arg)
{
    ConsumerData* data = static_cast<ConsumerData*>(arg);

    while (data->sem->take(500))
    {
        ++data->consumedCount;
    }

    return nullptr;
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <nbConsumers> <nbProducers> <tokensPerProducer>" << std::endl;
        return 1;
    }

    int nbConsumers = std::atoi(argv[1]);
    int nbProducers = std::atoi(argv[2]);
    unsigned long tokensPerProducer = std::strtoul(argv[3], nullptr, 10);

    if (nbConsumers <= 0 || nbProducers <= 0)
    {
        std::cerr << "nbConsumers and nbProducers must be > 0" << std::endl;
        return 1;
    }

    Semaphore sem(0);

    std::vector<ProducerData> producers(nbProducers);
    std::vector<ConsumerData> consumers(nbConsumers);
    std::vector<pthread_t> producerThreads(nbProducers);
    std::vector<pthread_t> consumerThreads(nbConsumers);

    for (int i = 0; i < nbConsumers; ++i)
    {
        consumers[i].id = i + 1;
        consumers[i].sem = &sem;
        consumers[i].consumedCount = 0;
    }

    for (int i = 0; i < nbProducers; ++i)
    {
        producers[i].id = i + 1;
        producers[i].sem = &sem;
        producers[i].tokensToProduce = tokensPerProducer;
        producers[i].producedCount = 0;
    }

    for (int i = 0; i < nbConsumers; ++i)
    {
        pthread_create(&consumerThreads[i], nullptr, consumer_run, &consumers[i]);
    }

    for (int i = 0; i < nbProducers; ++i)
    {
        pthread_create(&producerThreads[i], nullptr, producer_run, &producers[i]);
    }

    for (int i = 0; i < nbProducers; ++i)
    {
        pthread_join(producerThreads[i], nullptr);
    }

    for (int i = 0; i < nbConsumers; ++i)
    {
        pthread_join(consumerThreads[i], nullptr);
    }

    unsigned long totalProduced = 0;
    unsigned long totalConsumed = 0;

    std::cout << "=== Producers ===" << std::endl;
    for (int i = 0; i < nbProducers; ++i)
    {
        std::cout << "Producer " << producers[i].id
                  << " produced " << producers[i].producedCount << " tokens" << std::endl;
        totalProduced += producers[i].producedCount;
    }

    std::cout << "=== Consumers ===" << std::endl;
    for (int i = 0; i < nbConsumers; ++i)
    {
        std::cout << "Consumer " << consumers[i].id
                  << " consumed " << consumers[i].consumedCount << " tokens" << std::endl;
        totalConsumed += consumers[i].consumedCount;
    }

    std::cout << "Total produced = " << totalProduced << std::endl;
    std::cout << "Total consumed = " << totalConsumed << std::endl;

    if (totalProduced == totalConsumed)
    {
        std::cout << "OK: no token lost." << std::endl;
    }
    else
    {
        std::cout << "ERROR: token mismatch." << std::endl;
    }

    return 0;
}