mkdir bin > NUL
mkdir obj > NUL
@cl /TC /nologo /wd4711 /wd4710 /wd4242 /wd4244 /D_CRT_SECURE_NO_WARNINGS /Fo.\obj\ /Wall /c src\mpc.c src\repl.c
@link /nologo .\obj\mpc.obj .\obj\repl.obj /out:.\bin\repl.exe
