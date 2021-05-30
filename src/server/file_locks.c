#include "server.h"

void file_reader_lock(file_t* file){
    safe_pthread_mutex_lock(&(file->order_mtx));
    safe_pthread_mutex_lock(&(file->file_mtx));

    while(file->n_writers > 0)
        safe_pthread_cond_wait(&(file->access_cond), &(file->file_mtx));
    
    file->n_readers++;
    safe_pthread_mutex_unlock(&(file->order_mtx));
    safe_pthread_mutex_unlock(&(file->file_mtx));
}

void file_reader_unlock(file_t* file){
    safe_pthread_mutex_lock(&(file->file_mtx));

    file->n_readers--;
    if(file->n_readers == 0)
        safe_pthread_cond_signal(&(file->access_cond));
    
    safe_pthread_mutex_unlock(&(file->file_mtx));
}

void file_writer_lock(file_t* file){
    safe_pthread_mutex_lock(&(file->order_mtx));
    safe_pthread_mutex_lock(&(file->file_mtx));

    while(file->n_writers > 0 || file->n_readers > 0)
        safe_pthread_cond_wait(&(file->access_cond), &(file->file_mtx));
    
    file->n_writers++;
    safe_pthread_mutex_unlock(&(file->order_mtx));
    safe_pthread_mutex_unlock(&(file->file_mtx));
}

void file_writer_unlock(file_t* file){
    safe_pthread_mutex_lock(&(file->file_mtx));

    file->n_writers--;
    safe_pthread_cond_signal(&(file->access_cond));
    
    safe_pthread_mutex_unlock(&(file->file_mtx));
}