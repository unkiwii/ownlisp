#!/bin/bash
cc -std=c99 -g -Wall src/builtins.c src/env.c src/eval.c src/main.c src/mpc.c src/parser.c src/utils.c src/val.c -ledit -o bin/lispy
