# CS211 AI Cache Predictor Makefile
#
# Builds:
#   Cache_predictor.out          from Cache_predictor.c
#   Cache_predictor_initial.out  from Cache_predictor_initial.c
#
# Common commands:
#   make          builds both predictors
#   make main     builds only Cache_predictor.out
#   make initial  builds only Cache_predictor_initial.out
#   make clean    removes both executables and generated text outputs

CC = gcc
CFLAGS = -Wall -Werror -O2

MAIN_SRC = Cache_predictor.c
INITIAL_SRC = Cache_predictor_initial.c

MAIN_OUT = Cache_predictor.out
INITIAL_OUT = Cache_predictor_initial.out

.PHONY: all main initial clean

all: main initial

main: $(MAIN_OUT)

initial: $(INITIAL_OUT)

$(MAIN_OUT): $(MAIN_SRC)
	$(CC) $(CFLAGS) $(MAIN_SRC) -o $(MAIN_OUT)

$(INITIAL_OUT): $(INITIAL_SRC)
	$(CC) $(CFLAGS) $(INITIAL_SRC) -o $(INITIAL_OUT)

clean:
	rm -f $(MAIN_OUT) $(INITIAL_OUT)
	rm -f cache_input.txt predictions.txt
	rm -f cache_input_final.txt predictions_final.txt
	rm -f build_log.txt
