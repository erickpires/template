This program is a C implementation of the template script written by Ian Brunelli.

The program receives a list of filenames as its standard parameters
for each of the filenames it search the template directory for a file
with the same extension and then makes a copy of this file in the 
destination directory.

A destination directory can be specified with -d <DIR>, otherwise, the
current dir will be used.
If the destination file already exists, the program aborts with an error
message. Unless -o is passed as argument.
If the template files have the string "???" and -r [STR] is passed as argument
the "???" is replaced with STR. If -r is the last argument and STR is omitted, 
the default behavior is to replace "???" with the filename in uppercase 
with all its non-alphanumeric characters replaced with '_' (for use with C header files).
