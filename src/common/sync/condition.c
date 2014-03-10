#include "condition.h"

int condition_init(condition_t *cond) {
    return pthread_cond_init(&cond->_cond, NULL);
}

int condition_destroy(condition_t *cond) {
    return pthread_cond_destroy(&cond->_cond);
}

int condition_wait(condition_t *cond, mutex_t *mutex) {
    return pthread_cond_wait(&cond->_cond, &mutex->_mutex);
}

int condition_signal(condition_t *cond) {
    return pthread_cond_signal(&cond->_cond);
}

int condition_broadcast(condition_t *cond) {
    return pthread_cond_broadcast(&cond->_cond);
}