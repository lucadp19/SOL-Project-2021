#include "server.h"

void* sig_handler_thread(void* arg){
    // install handlers and all 
    sig_handler_arg_t* sigh_arg = (sig_handler_arg_t*)arg;
    sigset_t* set_ptr = sigh_arg->set;
    int* pipe = sigh_arg->pipe;

    #ifdef DEBUG
        printf("Hello, I'm the handler thread! ");
        printf("Pipe w_endp: %d. ", pipe[W_ENDP]);
        if(set_ptr == NULL)
            printf("Set_ptr is NULL :(");
    #endif 

    while(true){
        int sig;
        int err = sigwait(set_ptr, &sig);
        if(err != 0){
            errno = err;
            perror("Error in sigwait");
            return NULL;
        }

        switch (sig) {
            case SIGINT:
            case SIGQUIT:
                // TODO: terminating now
                // terminate = true;
                printf("\nReceived signal, closing server!\n");
                fflush(stdout);
                // waking sleeping threads
                close(pipe[W_ENDP]);
                pipe[W_ENDP] = -1;
                return NULL;
            case SIGHUP:
                // TODO: terminating *gently*
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

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    if((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0) {
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
    sig_handler_arg_t sigh_arg;
    sigh_arg.set  = &mask;
    sigh_arg.pipe = pipe;

    if( (err = pthread_create(sig_handler_tid, NULL, sig_handler_thread, &sigh_arg)) != 0) {
        // perror("Error while creating sig_handler thread");
        errno = err;
        return -1;
    }
    
    return 0;
}