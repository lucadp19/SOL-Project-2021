# TODO List for LSO Project

## General
- Decide if general (non-API) functions should perror or only return the error code
- Check the consistency of error checking

## Server
- Maybe enum type for worker thread results
- Workers shouldn't terminate if there is a problem in satisfying a client's request. They should probably send the client's code to main thread to close the connection
- Implement files. They should have
    - name (aka pathname)
    - contents in a void* buffer
    - a long client_lock which describes which client is currently locking the file
    - a mutex file_mtx to operate on the file atomically
    - *if I have the time* a recently read/timestamp/idkwhat bit for a LRU-like policy (I plan on using a hashmap of files, so a FIFO order wouldn't make much sense)
    - maybe other things but I don't know
- Implement operations on files (like open/close/append/read etc)
- Implement 

## Client
- Validate CL arguments (-D should follow -w or things like that)
- Implement operations: which sequences of API call should be used for the several kinds of client requests?

## API
- Implement basically all API functions

## Tests
- Write both test1 and test2

## Util
- Both *hashthings* have a specific remove method.
    - Should a more general version be implemented at list level?
    - Should they return the contents of the removed node? Probably yes

# Optional things
- I'd like to implement logging if I manage