%macro STRING 2
     %1 db %2, 0
     %1_length equ $ - %1 - 1
%endmacro
