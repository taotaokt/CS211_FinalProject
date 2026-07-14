============================================================
Rutgers University
CS211 - Computer Architecture (Summer 2026)

AI-Assisted Cache Prediction Benchmark
CS211 Cache Benchmark Suite (CCBS)

Version: 3.4.3
Build: 2026.07.08
============================================================

Directory Structure
-------------------
cache_simulator.exe
README.txt
VERSION.txt
pa3/
    Cache_predictor.c
    Cache_predictor_initial.c
    Makefile
    config_simulator.ini
    results/

Commands
--------
Practice:
    cache_simulator.exe --group 3 --mode practice --time 30

Benchmark:
    cache_simulator.exe --group 3 --mode benchmark --time 180 --run 1

Eval:
    cache_simulator.exe --group 3 --mode eval

Version:
    cache_simulator.exe --version

Makefile
--------
make          builds both predictors
make main     builds Cache_predictor.out
make initial  builds Cache_predictor_initial.out
make clean    removes generated executables and text outputs

Config File
-----------
pa3/config_simulator.ini is optional.

Priority:
    1. Command-line input
    2. config_simulator.ini
    3. Built-in defaults


Benchmark Run 4 iLab requirement
--------------------------------
Benchmark Run 4 must be run on an approved Rutgers iLab/CS machine.
The simulator checks the machine hostname before allowing:

    --mode benchmark --run 4

The simulator records:
    machine hostname
    fully qualified domain name
    platform
    iLab verification result

Required file checks
--------------------
The simulator checks the pa3 folder before running.

Practice/benchmark require:
    pa3/Cache_predictor.c
    pa3/Makefile

Eval/grading require:
    pa3/Cache_predictor.c
    pa3/Cache_predictor_initial.c
    pa3/Makefile

If any required file is missing, the simulator stops before building or running the predictors.
