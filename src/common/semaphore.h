#pragma once
#include <semaphore.h>
#include <string>
#include <chrono>

/**
 * @brief POSIX semaphore wrapper for interprocess synchronization.
 *
 * Current state:
 * - Producer: creates semaphore with initial value (O_CREAT, mode 0640)
 * - Consumer: waits up to `retries` × `delay_ms` for semaphore to appear
 * - Deletion: `sem_unlink()` called by Producer in destructor only
 *
 * @param name      Semaphore name (e.g. "/sem_empty")
 * @param value     Initial value (Producer only)
 * @param is_producer true = create, false = open existing with retries
 * @param retries   Max attempts for Consumer (default 50)
 * @param delay_ms  Delay between attempts in ms (default 100)
 */
class Semaphore {
public:
    Semaphore(const std::string &name, unsigned int value,
              bool is_producer,
              int retries = 50, int delay_ms = 100);

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