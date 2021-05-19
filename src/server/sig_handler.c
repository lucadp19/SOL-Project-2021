#include "server.h"

sigset_t sig_mask;

void* sig_handler_thread(void* arg){
    int* pipe = (int*)arg;

    #ifdef DEBUG
        printf("Hello, I'm the handler thread!\n");
    #endif 

    while(true){
        int sig;
        int err = sigwait(&sig_mask, &sig);
        if(err != 0){
            errno = err;
            perror("Error in sigwait");
            return NULL;
        }

        switch (sig) {
            // closing server immediately
            case SIGINT:
            case SIGQUIT:
                mode = CLOSE_SERVER;

                printf("\nReceived signal, closing server!\n");
                fflush(stdout);

                // signaling to main thread
                close(pipe[W_ENDP]);
                pipe[W_ENDP] = -1;

                return NULL;
            
            // blocking new connections
            case SIGHUP:
                mode = REFUSE_CONN;

                // signaling to main thread
                close(pipe[W_ENDP]);
                pipe[W_ENDP] = -1;
                return NULL;

            default: ;
        }
    }
    return NULL;
}

int install_sig_handler(int* pipe, pthread_t* sig_handler_tid){
    int err;

    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGINT);
    sigaddset(&sig_mask, SIGQUIT);
    sigaddset(&sig_mask, SIGHUP);

    if((err = pthread_sigmask(SIG_BLOCK, &sig_mask, NULL)) != 0) {
        errno = err;
        return -1;
    }

    #ifdef DEBUG
        printf("Masked SIGINT and SIGQUIT\n");
    #endif

    // Ignoring SIGPIPE
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = SIG_IGN;
    if( (sigaction(SIGPIPE, &sig_act, NULL) == -1)){
        // perror("Error while trying to ignore SIGPIPE");
        return -1;
    }

    #ifdef DEBUG
        printf("Ignored SIGPIPE\n");
        fflush(stdout);
    #endif

    // ------------ SIGHANDLER THREAD ------------ //
    if( (err = pthread_create(sig_handler_tid, NULL, sig_handler_thread, pipe)) != 0) {
        // perror("Error while creating sig_handler thread");
        errno = err;
        return -1;
    }
    
    return 0;
}