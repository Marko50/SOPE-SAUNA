all: generator sauna

generator: generator.c mainGenerator.c
	gcc generator.c mainGenerator.c -o generator -pthread -Wall
sauna: sauna.c mainSauna.c
	gcc sauna.c mainSauna.c -o sauna -Wall -pthread -lm
