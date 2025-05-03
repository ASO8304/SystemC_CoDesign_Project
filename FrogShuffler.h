#ifndef FROG_SHUFFLER_H
#define FROG_SHUFFLER_H

#include <systemc.h>
#include <vector>
#include <algorithm>
#include "FrogEvaluator.h"
using namespace std;

SC_MODULE(FrogShuffler) {
    // Inputs: Four memeplexes, each with 5 frogs
    sc_vector<sc_in<Frog>> memeplexe1_in;
    sc_vector<sc_in<Frog>> memeplexe2_in;
    sc_vector<sc_in<Frog>> memeplexe3_in;
    sc_vector<sc_in<Frog>> memeplexe4_in;

    // Output: Sorted frogs
    sc_vector<sc_out<Frog>> sorted_frogs_out;

    // Trigger signal
    sc_in<bool> shuffle_trigger;

    void combine_and_sort_frogs() {
        vector<Frog> combined_frogs(20);

        // Read frogs from memeplex inputs
        for (int i = 0; i < 5; ++i) {
            combined_frogs[i] = memeplexe1_in[i].read();
            combined_frogs[i + 5] = memeplexe2_in[i].read();
            combined_frogs[i + 10] = memeplexe3_in[i].read();
            combined_frogs[i + 15] = memeplexe4_in[i].read();
        }

        // Sort frogs in descending order based on fitness
        sort(combined_frogs.begin(), combined_frogs.end(), [](const Frog& a, const Frog& b) {
            return a.fitness > b.fitness;
            });

        // Write sorted frogs to outputs
        for (int i = 0; i < 20; ++i) {
            sorted_frogs_out[i].write(combined_frogs[i]);
        }
    }

    // Constructor
    SC_CTOR(FrogShuffler)
        : memeplexe1_in("memeplexe1_in", 5),
        memeplexe2_in("memeplexe2_in", 5),
        memeplexe3_in("memeplexe3_in", 5),
        memeplexe4_in("memeplexe4_in", 5),
        sorted_frogs_out("sorted_frogs_out", 20),
        shuffle_trigger("shuffle_trigger") {
        SC_METHOD(combine_and_sort_frogs);
        sensitive << shuffle_trigger; // Trigger shuffling only when shuffle_trigger is activated
        dont_initialize();
    }
};

#endif