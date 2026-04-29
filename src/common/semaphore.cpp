#include "semaphore.h"
#include <fcntl.h>
#include <thread>

Semaphore::Semaphore(const std::string &name, unsigned int value,
                     bool is_producer,
                     int retries, int delay_ms)
    : name_(name), is_producer_(is_producer) {
    if (is_producer) {
        sem_ = sem_open(name.c_str(), O_CREAT, 0640, value);
        if (sem_ == SEM_FAILED) {
            throw std::runtime_error("Failed to create semaphore: " + name);
        }
    } else {
        for (int i = 0; i < retries; ++i) {
            sem_ = sem_open(name.c_str(), 0);
            if (sem_ != SEM_FAILED) break;

            if (i == retries - 1) {
                throw std::runtime_error("Semaphore not found: " + name);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }
    }
}
