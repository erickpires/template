#! /bin/bash

# Should be done only if working_dir don't exists
# ln -s $PWD /tmp/working_dir 2> /dev/null

gcc -DLINUX=1 template.c string_buffer.c -o template -O3 -Wall && cp template ~/.local/bin/template

