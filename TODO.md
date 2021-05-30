# TODO List for LSO Project

## General
- CLEAN UP IT'S A MESS
- Decide if general (non-API) functions should perror or only return the error code
- Check the consistency of error checking

## Server
- Before read/write/append/everything one should have opened the file: check it!
- read_file: check client opened it
- write_file: check client opened it
- append_to_file: check client opened it and check the size
- remove_file: check client opened it

## Client
- Clean up code and check errors from API

## API
- Clean up API
- Decide which errno should be set when writen/readn fails
- Check if pathname/dirname/parameters are NULL
- Create an api_perror function!

## Tests
- Write test2

# Optional things
- statistiche.sh