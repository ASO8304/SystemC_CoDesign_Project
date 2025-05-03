#ifndef FROG_LOCAL_SEARCH_H
#define FROG_LOCAL_SEARCH_H

#include <systemc.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include "FrogEvaluator.h"
using namespace std;

SC_MODULE(FrogLocalSearch) {
    sc_vector<sc_in<Frog>> memeplex_in;
    sc_vector<sc_out<Frog>> updated_memeplex_out;
    sc_in<int> max_capacity;
    sc_vector<sc_in<int>> item_values;
    sc_vector<sc_in<int>> item_weights;
    sc_in<bool> ls_trigger;

    // Hardcoded Smax
    const double Smax = 2.0; // Maximum step size for local search

    void local_search() {
        const int frogs_per_memeplex = 5; // Number of solutions in the memeplex
        const int q = 3; // Number of solutions to select based on probability

        // Read the memeplex from input
        vector<Frog> memeplex(frogs_per_memeplex);
        for (int i = 0; i < frogs_per_memeplex; i++) {
            memeplex[i] = memeplex_in[i].read();
        }

        // Sort the memeplex by fitness in descending order
        sort(memeplex.begin(), memeplex.end(), [](const Frog& a, const Frog& b) {
            return a.fitness > b.fitness;
            });

        // Calculate weighted probabilities for each solution
        vector<double> probabilities(frogs_per_memeplex);
        double n = frogs_per_memeplex; // Number of solutions in the memeplex
        double sum_weights = n * (n + 1); // Sum of weights for normalization
        for (int j = 0; j < frogs_per_memeplex; j++) {
            probabilities[j] = (2 * (n + 1 - (j + 1))) / sum_weights;
        }

        // Select q solutions based on their probabilities
        random_device rd;
        mt19937 gen(rd());
        discrete_distribution<int> distribution(probabilities.begin(), probabilities.end());
        vector<Frog> selected_frogs;
        vector<int> selected_indices;
        while (selected_frogs.size() < q) {
            int idx = distribution(gen); // Select an index based on probability
            if (find(selected_indices.begin(), selected_indices.end(), idx) == selected_indices.end()) {
                selected_indices.push_back(idx); // Ensure no duplicate selections
                selected_frogs.push_back(memeplex[idx]);
            }
        }

        // Find the worst solution in the selected subset
        auto worst_it = min_element(selected_frogs.begin(), selected_frogs.end(), [](const Frog& a, const Frog& b) {
            return a.fitness < b.fitness;
            });
        int worst_index = selected_indices[distance(selected_frogs.begin(), worst_it)];
        Frog& worst_frog = memeplex[worst_index];
        Frog best_frog = memeplex.front(); // Best solution in the entire memeplex

        // Attempt to improve the worst solution
        bool improved = jump(worst_frog, best_frog, false);
        if (!improved) {
            jump(worst_frog, best_frog, true);
        }

        // If no improvement, generate a new random solution
        if (worst_frog.fitness <= memeplex[worst_index].fitness) {
            for (size_t k = 0; k < worst_frog.position.size(); k++) {
                worst_frog.position[k] = rand() % 2; // Randomly set position to 0 or 1
            }
            worst_frog.fitness = calculateFitness(worst_frog.position);
        }

        // Update the memeplex with the optimized solution
        memeplex[worst_index] = worst_frog;

        // Write the updated memeplex to output
        for (int i = 0; i < frogs_per_memeplex; i++) {
            updated_memeplex_out[i].write(memeplex[i]);
        }
    }

    SC_CTOR(FrogLocalSearch)
        : memeplex_in("memeplex_in", 5),
        updated_memeplex_out("updated_memeplex_out", 5),
        item_values("item_values", 9),
        item_weights("item_weights", 9),
        ls_trigger("ls_trigger") {
        SC_METHOD(local_search);
        sensitive << ls_trigger;
        dont_initialize();
    }

private:
    int calculateFitness(const vector<int>&position) {
        int total_value = 0;
        int total_weight = 0;
        int max_cap = max_capacity.read();
        for (size_t i = 0; i < position.size(); i++) {
            if (position[i] == 1) {
                total_value += item_values[i].read();
                total_weight += item_weights[i].read();
            }
        }
        return (total_weight > max_cap) ? 0 : total_value;
    }

    double generateRandom() {
        return double(rand()) / RAND_MAX;
    }

    bool jump(Frog & frogW, const Frog & frogB, bool negativeStep) {
        int oldFitness = frogW.fitness;

        // Gather diff bits
        vector<int> diffIndices;
        diffIndices.reserve(frogW.position.size());
        if (!negativeStep) {
            // Positive ==> flip 0->1 if B=1
            for (size_t i = 0; i < frogW.position.size(); i++) {
                if (!frogW.position[i] && frogB.position[i]) {
                    diffIndices.push_back(i);
                }
            }
        }
        else {
            // Negative ==> flip 1->0 if B=0
            for (size_t i = 0; i < frogW.position.size(); i++) {
                if (frogW.position[i] && !frogB.position[i]) {
                    diffIndices.push_back(i);
                }
            }
        }

        int distance = diffIndices.size();
        if (distance == 0) {
            return false;
        }

        double r = generateRandom();
        double rawStep = r * distance;
        double stepSize = 0.0;

        if (!negativeStep) {
            // S = min[r * distance, Smax]
            stepSize = min(rawStep, Smax); // Use hardcoded Smax
        }
        else {
            // S = max[r * distance, -Smax]
            stepSize = max(-rawStep, -Smax); // Use hardcoded Smax
        }

        int nFlips = static_cast<int>(floor(fabs(stepSize)));
        if (nFlips <= 0) {
            return false;
        }
        if (nFlips > distance) {
            nFlips = distance;
        }

        static random_device rd;
        static mt19937 g(rd());
        shuffle(diffIndices.begin(), diffIndices.end(), g);

        for (int i = 0; i < nFlips; i++) {
            int bitPos = diffIndices[i];
            frogW.position[bitPos] = !frogW.position[bitPos];
        }

        frogW.fitness = calculateFitness(frogW.position);
        return (frogW.fitness > oldFitness);
    }
};

#endif