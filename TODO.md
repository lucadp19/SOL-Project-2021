# TODO List for LSO Project

## General
- Decide if general (non-API) functions should perror or only return the error code

## Server
- Maybe enum type for worker thread results

## Client
- Implement other operations
- Maybe read time increment from arguments (option -w for wait)...

## Util
- Both *hashthings* have a specific remove method.
    - Should a more general version be implemented at list level?
    - Should they return the contents of the removed node? Probably yes