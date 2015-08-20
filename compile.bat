@mkdir bin >NUL 2>&1
@mkdir obj >NUL 2>&1
@cl /TC /nologo /wd4100 /wd4127 /wd4711 /wd4710 /wd4242 /wd4244 /wd4820 /D_CRT_SECURE_NO_WARNINGS /Fo.\obj\ /Wall /c src\builtins.c src\env.c src\eval.c src\main.c src\mpc.c src\parser.c src\utils.c src\val.c
@link /nologo .\obj\builtins.obj .\obj\env.obj .\obj\eval.obj .\obj\main.obj .\obj\mpc.obj .\obj\parser.obj .\obj\utils.obj .\obj\val.obj /out:.\bin\lispy.exe
