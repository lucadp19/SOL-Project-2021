#include "server.h"
#include <stdarg.h>

int init_log_file(){
    struct tm curr_time;
    char* log_file_path;
    time_t curr_time_abs = time(NULL);

    localtime_r(&curr_time_abs, &curr_time);
    int dir_path_len = strlen(server_config.log_dir_path);

    log_file_path = safe_malloc((dir_path_len + 30) * sizeof(char));
    memset(log_file_path, '\0', sizeof(char)*(dir_path_len + 30));
    // writing info on log_file_path string
    snprintf(
        log_file_path, dir_path_len + 30,
        "%s/log-%4d-%02d-%02d-%02d:%02d:%02d.txt", 
        server_config.log_dir_path,
        curr_time.tm_year + 1900,
        curr_time.tm_mon + 1,
        curr_time.tm_mday,
        curr_time.tm_hour,
        curr_time.tm_min,
        curr_time.tm_sec
    );

    debug("Log file path is: %s\n", log_file_path);
    if( (log_file = fopen(log_file_path, "w")) == NULL){
        fprintf(stderr, "Fatal error in creating log file. Aborting.\n");
        return -1;
    }
    free(log_file_path);

    return 0;
}

void logger(const char* fmt, ...){
    time_t abstime;
    struct tm struct_time;

    va_list arg_list;
    va_start(arg_list, fmt);

    safe_pthread_mutex_lock(&log_file_mtx);
    abstime = time(NULL);
    localtime_r(&abstime, &struct_time);

    // printing time
    fprintf(
        log_file, 
        "%4d-%02d-%02d %02d:%02d:%02d  ",
        struct_time.tm_year + 1900,
        struct_time.tm_mon + 1,
        struct_time.tm_mday,
        struct_time.tm_hour,
        struct_time.tm_min,
        struct_time.tm_sec
    );
    // printing the given string
    vfprintf(log_file, fmt, arg_list);
    fflush(log_file);

    safe_pthread_mutex_unlock(&log_file_mtx);
}