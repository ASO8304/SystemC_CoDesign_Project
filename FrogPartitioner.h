#ifndef FROG_PARTITIONER_H
#define FROG_PARTITIONER_H

#include <systemc.h>
#include <vector>
#include "FrogEvaluator.h"
using namespace std;

SC_MODULE(FrogPartitioner) {
    // Input: 20 sorted frogs
    sc_vector<sc_in<Frog>> sorted_frogs_in;

    // Outputs: Four memeplexes, each with 5 frogs
    sc_vector<sc_out<Frog>> memeplexe1_out;
    sc_vector<sc_out<Frog>> memeplexe2_out;
    sc_vector<sc_out<Frog>> memeplexe3_out;
    sc_vector<sc_out<Frog>> memeplexe4_out;

    // Control signal to trigger partitioning
    sc_in<bool> partition_trigger;

    void partition_frogs() {
        const int M = 4;  // Number of memeplexes
        const int N = 20; // Total number of frogs
        const int frogs_per_memeplex = N / M;

        // Vectors to store frogs for each memeplex
        vector<Frog> memeplex1;
        vector<Frog> memeplex2;
        vector<Frog> memeplex3;
        vector<Frog> memeplex4;

        // Distribute frogs into memeplexes
        for (int i = 0; i < N; i++) {
            Frog frog = sorted_frogs_in[i].read();
            int memeplex_index = i % M;
            switch (memeplex_index) {
            case 0:
                memeplex1.push_back(frog);
                break;
            case 1:
                memeplex2.push_back(frog);
                break;
            case 2:
                memeplex3.push_back(frog);
                break;
            case 3:
                memeplex4.push_back(frog);
                break;
            }
        }

        // Write frogs to their respective memeplex outputs
        for (int i = 0; i < frogs_per_memeplex; i++) {
            memeplexe1_out[i].write(memeplex1[i]);
            memeplexe2_out[i].write(memeplex2[i]);
            memeplexe3_out[i].write(memeplex3[i]);
            memeplexe4_out[i].write(memeplex4[i]);
        }

        // Print the memeplex assignments for debugging purposes
        /*cout << "Memeplex 1:" << endl;
        for (const auto& frog : memeplex1) {
            cout << "Position: ";
            for (int j = 0; j < frog.position.size(); ++j) {
                cout << frog.position[j];
            }
            cout << ", Fitness: " << frog.fitness << endl;
        }

        cout << "Memeplex 2:" << endl;
        for (const auto& frog : memeplex2) {
            cout << "Position: ";
            for (int j = 0; j < frog.position.size(); ++j) {
                cout << frog.position[j];
            }
            cout << ", Fitness: " << frog.fitness << endl;
        }

        cout << "Memeplex 3:" << endl;
        for (const auto& frog : memeplex3) {
            cout << "Position: ";
            for (int j = 0; j < frog.position.size(); ++j) {
                cout << frog.position[j];
            }
            cout << ", Fitness: " << frog.fitness << endl;
        }

        cout << "Memeplex 4:" << endl;
        for (const auto& frog : memeplex4) {
            cout << "Position: ";
            for (int j = 0; j < frog.position.size(); ++j) {
                cout << frog.position[j];
            }
            cout << ", Fitness: " << frog.fitness << endl;
        }*/
    }

    // Constructor
    SC_CTOR(FrogPartitioner)
        : sorted_frogs_in("sorted_frogs_in", 20),
        memeplexe1_out("memeplexe1_out", 5),
        memeplexe2_out("memeplexe2_out", 5),
        memeplexe3_out("memeplexe3_out", 5),
        memeplexe4_out("memeplexe4_out", 5),
        partition_trigger("partition_trigger") {
        // Use SC_METHOD and make it sensitive to the partition_trigger signal
        SC_METHOD(partition_frogs);
        sensitive << partition_trigger; // Trigger partitioning only when partition_trigger is activated
        dont_initialize(); // Prevent automatic initialization of sensitive processes
    }
};

#endif