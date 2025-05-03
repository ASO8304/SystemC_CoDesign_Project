#ifndef FROG_EVALUATOR_H
#define FROG_EVALUATOR_H

#include <systemc.h>
#include <vector>
using namespace std;

struct Frog {
    vector<int> position;
    int fitness;
    bool operator==(const Frog& other) const {
        return position == other.position && fitness == other.fitness;
    }
    friend ostream& operator<<(ostream& os, const Frog& f) {
        os << "[";
        for (size_t i = 0; i < f.position.size(); i++) {
            os << f.position[i] << (i < f.position.size() - 1 ? ", " : "");
        }
        os << "] Fitness: " << f.fitness;
        return os;
    }
};

namespace sc_core {
    inline void sc_trace(sc_trace_file* tf, const Frog& f, const std::string& name) {
        for (size_t i = 0; i < f.position.size(); i++) {
            sc_trace(tf, f.position[i], name + "_pos" + to_string(i));
        }
        sc_trace(tf, f.fitness, name + "_fitness");
    }
}

SC_MODULE(FrogEvaluator) {
    // Inputs and Outputs
    sc_vector<sc_in<Frog>> frogs_in;
    sc_vector<sc_out<Frog>> frogs_out;

    // Control signals
    sc_in<int> max_capacity;
    sc_vector<sc_in<int>> prices;
    sc_vector<sc_in<int>> weights;
    sc_in<int> dimension;

    // Trigger signal
    sc_in<bool> evaluate_trigger;

    // Method to evaluate fitness
    void evaluate_fitness() {
        int dim = dimension.read();
        vector<int> price_vec(dim, 0);
        vector<int> weight_vec(dim, 0);

        for (int i = 0; i < dim; i++) {
            price_vec[i] = prices[i].read();
            weight_vec[i] = weights[i].read();
        }

        int max_cap = max_capacity.read();
        for (int i = 0; i < 20; i++) {
            Frog f = frogs_in[i].read();
            int total_value = 0, total_weight = 0;

            for (int j = 0; j < f.position.size(); j++) {
                if (f.position[j] == 1) {
                    total_value += price_vec[j];
                    total_weight += weight_vec[j];
                }
            }

            f.fitness = (total_weight > max_cap) ? 0 : total_value;
            frogs_out[i].write(f);
        }
    }

    // Constructor
    SC_CTOR(FrogEvaluator)
        : frogs_in("frogs_in", 20),
        frogs_out("frogs_out", 20),
        prices("prices", 9),
        weights("weights", 9),
        evaluate_trigger("evaluate_trigger") {
        SC_METHOD(evaluate_fitness);
        sensitive << evaluate_trigger; // Trigger evaluation only when evaluate_trigger is activated
        dont_initialize();
    }
};

#endif