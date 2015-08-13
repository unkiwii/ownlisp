@mkdir bin >NUL 2>&1
@mkdir obj >NUL 2>&1
@cl /TC /nologo /wd4127 /wd4711 /wd4710 /wd4242 /wd4244 /wd4820 /D_CRT_SECURE_NO_WARNINGS /Fo.\obj\ /Wall /c src\mpc.c src\repl.c
@link /nologo .\obj\mpc.obj .\obj\repl.obj /out:.\bin\repl.exe
