/*
 *  Mutex Wrapper Class
 *
 *  Modified from the OpenKinectProject
 *  Used under the terms of the Apache License v2.0 and GPL2
 *  See the following URLs for license details:
 *  http://www.apache.org/licenses/LICENSE-2.0
 *  http://www.gnu.org/licenses/gpl-2.0.txt
 *
 *  Wrapper for pthread mutexes and locks for easier use in C++
 *  Taylor O'Brien
 *  14 July 2011
 */
 
#include "includes/mutex.h"
 
Mutex::Mutex() {
    pthread_mutex_init(&_mmutex, NULL);
}

bool Mutex::lock() {
    return pthread_mutex_lock( &_mmutex ) == 0;
}

bool Mutex::unlock() {
    return pthread_mutex_unlock( &_mmutex ) == 0;
}

Mutex::Lock::Lock(Mutex & mutex) : _mutex(mutex) {
    _mutex.lock();
}

Mutex::Lock::~Lock() {
    _mutex.unlock();
}
 