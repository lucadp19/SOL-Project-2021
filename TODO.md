# TODO List for LSO Project

## General
- CLEAN UP IT'S A MESS
- Decide if general (non-API) functions should perror or only return the error code
- Check the consistency of error checking

## Server
- Before read/write/append/everything one should have opened the file: check it!

## Client
- Validate CL arguments (-D should follow -w or things like that)
- Implement operations: which sequences of API call should be used for the several kinds of client requests?

## API
- Clean up API
- Decide which errno should be set when writen/readn fails
- Check if pathname/dirname/parameters are NULL
- Write/other functions should check if dirname is NULL and destroy files

## Tests
- Write both test1 and test2

## Util
- Both *hashthings* have a specific remove method.
    - Should a more general version be implemented at list level?
    - Should they return the contents of the removed node? Probably yes

# Optional things