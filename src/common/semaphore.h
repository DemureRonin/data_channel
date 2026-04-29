#pragma once
#include <semaphore.h>
#include <string>
#include <chrono>

/**
 * @brief POSIX semaphore wrapper for interprocess synchronization.
 *
 * Producer: creates semaphore with initial value
 * Consumer: waits for semaphore to exist, then opens it
 */
class Semaphore {
public:
    /**
     * @param name      Semaphore name (e.g. "/sem_empty")
     * @param value     Initial value (used only for Producer)
     * @param is_producer true = create semaphore, false = wait & open existing & unlink
     * @param retries   Max wait attempts for Consumer (default 10000)
     * @param delay_ms  Delay between attempts in ms (default 100)
     */
    Semaphore(const std::string &name, unsigned int value,
              bool is_producer,
              int retries = 10, int delay_ms = 100);

    void Wait() const { sem_wait(sem_); }
    void Post() const { sem_post(sem_); }

    ~Semaphore() {
        if (sem_ != SEM_FAILED) sem_close(sem_);
        if (is_producer_) sem_unlink(name_.c_str());
    }

private:
    std::string name_;
    sem_t *sem_;
    bool is_producer_;
};
