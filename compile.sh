#!/bin/bash
cc -std=c99 -Wall src/mpc.c src/repl.c -ledit -o bin/repl
