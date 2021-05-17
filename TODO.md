# TODO List for LSO Project

## General
- Create a macro to print to stdout only if DEBUG is defined (instead of writing `#ifdef DEBUG`, `#endif` every time)
- Decide if general (non-API) functions should perror or only return the error code

## Server
- Modify the config structure and function to use a "key : value" structure instead of having to write them in order
- Socket select + dealing with client connections (hashtable) + dealing with pipes and closing things properly
- Enum type to describe server state (continue/closing/closing *gently*)
