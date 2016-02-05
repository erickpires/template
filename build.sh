#! /bin/bash

# Should be done only if working_dir don't exists
# ln -s $PWD /tmp/working_dir 2> /dev/null

gcc -DLINUX=1 -g template.c -o main -O3 -Wall -Wno-unused-but-set-variable && cp main ~/.local/bin/my_template

