#!/bin/bash
cc -std=c99 -g -Wall src/mpc.c src/repl.c -ledit -o bin/repl
