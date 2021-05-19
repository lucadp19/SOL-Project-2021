#!/bin/bash

if [ ! -d bin ]                                         \
    || [ ! -d obj/util ]  || [ ! -d obj/util/hash ]     \
    || [ ! -d obj/api ]                                 \
    || [ ! -d obj/server ]                              \
    || [ ! -d obj/client ]
then
    echo "Creating bin/ and obj/ directories..."
    mkdir -p bin/ obj/util obj/util/hash obj/api obj/server obj/client
    echo "Directories created!"
fi