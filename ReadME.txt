Shuffled Frog Leaping Algorithm (SFLA) Implementation
-----------------------------------------------------

Author:
--------
Abolfazl Sheikhoveisi


Date:
-----
2/5/2025

Description:
------------
This project implements the Shuffled Frog Leaping Algorithm (SFLA), a metaheuristic optimization algorithm inspired by the behavior of frogs leaping in a pond. The algorithm is designed to solve the 0-1 Knapsack Problem, by iteratively improving a population of candidate solutions (frogs).

The implementation is written in C++ using SystemC, a library for system-level design and modeling. The code is modular and includes components for frog evaluation, sorting, partitioning, local search, and shuffling.

Contents:
---------
1. FrogLocalSearch.h   - Implements the local search module for improving individual frogs.
2. FrogEvaluator.h     - Contains the fitness evaluation logic for frogs.
3. FrogPartitioner.h   - Partitions the population into memeplexes.
4. FrogShuffler.h      - Shuffles the population after local search.
5. main.cpp            - Main driver program for the SFLA algorithm.
7. ControllerModule.m  - MATLAB code to control and iterates the algorithm.

Dependencies:
-------------
- SystemC library (version 3.0.1 or later)
- C++ compiler with C++17 support (e.g., g++, clang++, or MSVC)

Algorithm Overview:
-------------------
1. Initialize a population of frogs with random solutions.
2. Evaluate the fitness of each frog.
3. Sort the population in descending order of fitness.
4. Partition the population into memeplexes.
5. Perform local search within each memeplex to improve solutions.
6. Shuffle the population and repeat the process until a stopping criterion is met.

Key Features:
-------------
- Modular design for easy extension and customization.
- Weighted probability-based selection for local search.
- Configurable parameters for population size, memeplex count, and step size.
- Efficient fitness evaluation and local search mechanisms.

Contact:
--------
For questions or feedback, please contact:
- abolfazl.sheikhoveisi@gmail.com
- mohammadhoseinParvini@gmail.com
