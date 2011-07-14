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

#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

class Mutex {
private:
    pthread_mutex_t _mmutex;
public:
    Mutex();
    bool lock();
    bool unlock();
    
    class Lock {
    private:
        Mutex & _mutex;
    
    public:
        Lock(Mutex & mutex);
        ~Lock();
    };
};

#endif