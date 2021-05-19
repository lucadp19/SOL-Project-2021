# TODO List for LSO Project

## General
- Create a macro to print to stdout only if DEBUG is defined (instead of writing `#ifdef DEBUG`, `#endif` every time)
- Decide if general (non-API) functions should perror or only return the error code

## Server
- Modify the config structure and function to use a "key : value" structure instead of having to write them in order
- Maybe enum type for worker thread results

## Client
- Implement other operations
- Maybe read time increment from arguments (option -w for wait)...