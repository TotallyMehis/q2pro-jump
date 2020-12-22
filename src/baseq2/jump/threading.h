#pragma once

typedef void* (thread_cb)(void*);
typedef void (thread_ready_cb)(void*);


#ifdef __cplusplus
extern "C" void Jump_Thread_Create(thread_cb cb, thread_ready_cb ready_cb, void *data, int delete_data);

extern "C" void Jump_Thread_Update(void);

extern "C" void Jump_Thread_CloseAll(void);
#else
/// <summary>
/// Creates a detached thread.
/// </summary>
/// <param name="cb">Callback</param>
/// <param name="ready_cb">Called back on main thread when done doing work in the thread.</param>
/// <param name="data">Data to pass to the callback. MAKE SURE THIS IS ALLOCATED WITH gi.TagMalloc!!!</param>
/// <param name="delete_data">Is the 'data' parameter automatically deleted when done running thread.</param>
void Jump_Thread_Create(thread_cb cb, thread_ready_cb ready_cb, void *data, int delete_data);

void Jump_Thread_Update(void);

void Jump_Thread_CloseAll(void);
#endif
