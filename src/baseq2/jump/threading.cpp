#include <atomic>
#include <thread>
#include <vector>
#include <functional>

#include "threading.h"

extern "C"
{
    #include "g_local.h"
}


// simple struct that holds per thread data.
struct mythread_t {
    // actual thread object
    std::thread *thr;

    // atomic boolean to signify when done.
    std::atomic<bool> *done;

    // the callback called on main thread once done.
    thread_ready_cb *ready_cb;
    // data to pass to the callback. we will free it.
    void *out_data;
    
    // the identifier
    int thread_id;
};

static std::vector<mythread_t> threads;
static int inc_thread_id = 0;





static void MarkThreadDone(int thread_id, void *out_data)
{
    for (std::vector<mythread_t>::iterator it = threads.begin(); it != threads.end(); it++) {
        if (it->thread_id == thread_id) {
            *it->done = true;
            it->out_data = out_data;
            break;
        }
    }
}

void Jump_Thread_Create(thread_cb cb, thread_ready_cb ready_cb, void* data, int delete_data)
{
    threads.push_back({
        nullptr,
        new std::atomic<bool>(false),
        ready_cb,
        nullptr,
        ++inc_thread_id
    });

    auto& newthr = threads[threads.size() - 1];


    int my_thread_id = newthr.thread_id;
    std::function<void()> inner_cb = [my_thread_id, cb, data, delete_data]() {
        void *out_data = cb(data);

        MarkThreadDone(my_thread_id, out_data);

        if (delete_data && data) {
            //delete data;
            gi.TagFree(data);
        }
    };

    

    newthr.thr = new std::thread(inner_cb);
    newthr.thr->detach();
}

void Jump_Thread_Update(void)
{
    for (std::vector<mythread_t>::iterator it = threads.begin(); it != threads.end();) {
        if (*it->done) {
            // join thread
            if (it->thr->joinable()) {
                it->thr->join();
            }

            // call ready callback
            if (it->ready_cb)
                it->ready_cb(it->out_data);
            
            if (it->out_data) {
                gi.TagFree(it->out_data);
            }

            delete it->thr;
            delete it->done;

            it = threads.erase(it);
        } else {
            ++it;
        }
    }
}

void Jump_Thread_CloseAll(void)
{
    while (threads.size() > 0) {
        Jump_Thread_Update();
    }
}
