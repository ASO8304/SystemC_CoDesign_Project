#include <systemc.h>
#include <fstream>
#include <sstream>
#include <fstream>
#include <vector>
#include "FrogEvaluator.h"
#include "FrogPartitioner.h"
#include "FrogLocalSearch.h"
#include "FrogShuffler.h"

SC_MODULE(SFLA_System) {
    // Signals
    sc_vector<sc_signal<Frog>> frog_population;
    sc_vector<sc_signal<Frog>> partitioned_memeplex1, partitioned_memeplex2, partitioned_memeplex3, partitioned_memeplex4;
    sc_vector<sc_signal<Frog>> ls_output1, ls_output2, ls_output3, ls_output4;
    sc_vector<sc_signal<Frog>> shuffled_frogs;

    // Control Signals
    sc_signal<int> max_capacity;
    sc_vector<sc_signal<int>> prices, weights;
    sc_signal<int> dimension;

    // Trigger Signals
    sc_signal<bool> evaluate_trigger;
    sc_signal<bool> partition_trigger;
    sc_signal<bool> ls1_trigger, ls2_trigger, ls3_trigger, ls4_trigger;
    sc_signal<bool> shuffle_trigger;

    // Events for synchronization
    sc_event evaluator_done, partitioner_done, ls1_done, ls2_done, ls3_done, ls4_done, shuffler_done;

    // Modules
    FrogEvaluator evaluator;
    FrogPartitioner partitioner;
    FrogLocalSearch ls1, ls2, ls3, ls4;
    FrogShuffler shuffler;

    // Function to read initialization values from CSV
    void initialize_parameters_from_csv(const std::string & filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            SC_REPORT_ERROR("SFLA_System", "Failed to open input CSV file.");
            return;
        }

        std::string line;
        std::vector<int> price_values, weight_values;

        // Read the CSV file line by line
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string cell;

            // Skip item ID
            std::getline(ss, cell, ',');

            // Extract price
            std::getline(ss, cell, ',');
            price_values.push_back(std::stoi(cell));

            // Extract weight
            std::getline(ss, cell, ',');
            weight_values.push_back(std::stoi(cell));
        }

        file.close();

        // Set knapsack capacity
        max_capacity.write(25);

        // Initialize item prices and weights
        for (size_t i = 0; i < price_values.size(); ++i) {
            prices[i].write(price_values[i]);
            weights[i].write(weight_values[i]);
        }

        dimension.write(static_cast<int>(price_values.size()));

        // Debugging: Print prices and weights
        cout << "Prices: ";
        for (int p : price_values) cout << p << " ";
        cout << endl;

        cout << "Weights: ";
        for (int w : weight_values) cout << w << " ";
        cout << endl;
    }

    void initialize_frogs_from_csv(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            SC_REPORT_ERROR("SFLA_System", "Failed to open input CSV file.");
            return;
        }

        std::string line;
        std::vector<Frog> frogs;
        int frog_count = 0;

        while (std::getline(file, line) && frog_count < 20) {
            std::stringstream ss(line);
            std::string cell;
            Frog f;
            f.position.clear();

            // Read binary position values
            for (int i = 0; i < 9; ++i) { // Assuming 9 items in knapsack
                std::getline(ss, cell, ',');
                f.position.push_back(std::stoi(cell));
            }

            // Read fitness value
            std::getline(ss, cell, ',');
            f.fitness = std::stoi(cell);

            frogs.push_back(f);
            frog_count++;
        }

        file.close();

        // Debugging
        /*std::cout << "Loaded frogs (before writing to signals):\n";
        for (int i = 0; i < frogs.size(); ++i) {
            std::cout << "Frog " << i << ": [";
            for (int bit : frogs[i].position) std::cout << bit;
            std::cout << "] Fitness: " << frogs[i].fitness << std::endl;
        }*/

        wait(SC_ZERO_TIME);
        // Write frogs to signals
        for (int i = 0; i < 20; ++i) {
            frog_population[i].write(frogs[i]);
        }
        wait(SC_ZERO_TIME);
        // Debugging: Print loaded frogs
        /*std::cout << "Loaded frog population from CSV:" << std::endl;
        for (int i = 0; i < 20; ++i) {
            Frog f = frog_population[i].read();
            std::cout << "Frog " << i << ": [";
            for (int bit : f.position) std::cout << bit;
            std::cout << "] Fitness: " << f.fitness << std::endl;
        }*/
    }

    SC_CTOR(SFLA_System)
        : frog_population("frog_population", 20),
        partitioned_memeplex1("memeplex1", 5),
        partitioned_memeplex2("memeplex2", 5),
        partitioned_memeplex3("memeplex3", 5),
        partitioned_memeplex4("memeplex4", 5),
        ls_output1("ls_out1", 5),
        ls_output2("ls_out2", 5),
        ls_output3("ls_out3", 5),
        ls_output4("ls_out4", 5),
        shuffled_frogs("shuffled", 20),
        prices("prices", 9),
        weights("weights", 9),
        evaluator("evaluator"),
        partitioner("partitioner"),
        ls1("ls1"), ls2("ls2"), ls3("ls3"), ls4("ls4"),
        shuffler("shuffler"),
        evaluate_trigger("evaluate_trigger"),
        partition_trigger("partition_trigger"),
        ls1_trigger("ls1_trigger"),
        ls2_trigger("ls2_trigger"),
        ls3_trigger("ls3_trigger"),
        ls4_trigger("ls4_trigger"),
        shuffle_trigger("shuffle_trigger")
    {
        // Connect evaluator
        evaluator.frogs_in.bind(frog_population);
        evaluator.frogs_out.bind(frog_population);
        evaluator.prices.bind(prices);
        evaluator.weights.bind(weights);
        evaluator.dimension.bind(dimension);
        evaluator.max_capacity.bind(max_capacity);
        evaluator.evaluate_trigger.bind(evaluate_trigger);

        // Connect partitioner
        partitioner.sorted_frogs_in.bind(frog_population);
        partitioner.memeplexe1_out.bind(partitioned_memeplex1);
        partitioner.memeplexe2_out.bind(partitioned_memeplex2);
        partitioner.memeplexe3_out.bind(partitioned_memeplex3);
        partitioner.memeplexe4_out.bind(partitioned_memeplex4);
        partitioner.partition_trigger.bind(partition_trigger);

        // Connect local search modules
        ls1.memeplex_in.bind(partitioned_memeplex1);
        ls1.updated_memeplex_out.bind(ls_output1);
        ls1.max_capacity.bind(max_capacity);
        ls1.item_values.bind(prices);
        ls1.item_weights.bind(weights);
        ls1.ls_trigger.bind(ls1_trigger);

        ls2.memeplex_in.bind(partitioned_memeplex2);
        ls2.updated_memeplex_out.bind(ls_output2);
        ls2.max_capacity.bind(max_capacity);
        ls2.item_values.bind(prices);
        ls2.item_weights.bind(weights);
        ls2.ls_trigger.bind(ls2_trigger);

        ls3.memeplex_in.bind(partitioned_memeplex3);
        ls3.updated_memeplex_out.bind(ls_output3);
        ls3.max_capacity.bind(max_capacity);
        ls3.item_values.bind(prices);
        ls3.item_weights.bind(weights);
        ls3.ls_trigger.bind(ls3_trigger);

        ls4.memeplex_in.bind(partitioned_memeplex4);
        ls4.updated_memeplex_out.bind(ls_output4);
        ls4.max_capacity.bind(max_capacity);
        ls4.item_values.bind(prices);
        ls4.item_weights.bind(weights);
        ls4.ls_trigger.bind(ls4_trigger);

        // Connect shuffler
        shuffler.memeplexe1_in.bind(ls_output1);
        shuffler.memeplexe2_in.bind(ls_output2);
        shuffler.memeplexe3_in.bind(ls_output3);
        shuffler.memeplexe4_in.bind(ls_output4);
        shuffler.sorted_frogs_out.bind(shuffled_frogs);
        shuffler.shuffle_trigger.bind(shuffle_trigger);

        // Initialize system
        SC_THREAD(initialize_parameters);
        SC_THREAD(run_evaluator);
        SC_THREAD(run_partitioner);
        SC_THREAD(run_local_search);
        SC_THREAD(run_shuffler);
        SC_THREAD(run_saver); 
    }


    void initialize_parameters() {
        // Read initialization values from CSV
        initialize_parameters_from_csv("input_parameters.csv");
        initialize_frogs_from_csv("frog_population.csv");
    }

    void run_saver() {
        wait(shuffler_done);
        saveShufflerOutput(shuffled_frogs, "final_result.csv");
    }


    void run_evaluator() {
        wait(SC_ZERO_TIME);
        evaluate_trigger.write(true); // Activate evaluator
        wait(SC_ZERO_TIME); // Allow evaluator to process
        evaluate_trigger.write(false); // Deactivate evaluator
        
        wait(SC_ZERO_TIME);
        // Debugging: Print evaluated frogs
        /*cout << "After evaluation:" << endl;
        for (int i = 0; i < 20; ++i) {
            Frog f = frog_population[i].read();
            cout << "Frog " << i << ": [";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << "] Fitness: " << f.fitness << endl;
        }*/

        evaluator_done.notify();
    }

    void run_partitioner() {
        wait(evaluator_done);
        partition_trigger.write(true); // Activate partitioner
        wait(SC_ZERO_TIME); // Allow partitioner to process
        partition_trigger.write(false); // Deactivate partitioner

        wait(SC_ZERO_TIME);
        // Debugging: Print memeplexes
        /*cout << "Memeplex 1:" << endl;
        for (int i = 0; i < 5; ++i) {
            Frog f = partitioned_memeplex1[i].read();
            cout << "Position: ";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << ", Fitness: " << f.fitness << endl;
        }

        cout << "Memeplex 2:" << endl;
        for (int i = 0; i < 5; ++i) {
            Frog f = partitioned_memeplex2[i].read();
            cout << "Position: ";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << ", Fitness: " << f.fitness << endl;
        }

        cout << "Memeplex 3:" << endl;
        for (int i = 0; i < 5; ++i) {
            Frog f = partitioned_memeplex3[i].read();
            cout << "Position: ";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << ", Fitness: " << f.fitness << endl;
        }

        cout << "Memeplex 4:" << endl;
        for (int i = 0; i < 5; ++i) {
            Frog f = partitioned_memeplex4[i].read();
            cout << "Position: ";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << ", Fitness: " << f.fitness << endl;
        }*/

        partitioner_done.notify();
    }

    void run_local_search() {
        wait(partitioner_done);
        ls1_trigger.write(true); // Activate LS1
        ls2_trigger.write(true);
        ls3_trigger.write(true); // Activate LS3
        ls4_trigger.write(true);

        wait(SC_ZERO_TIME); // Allow LS1 to process

        ls1_trigger.write(false); // Deactivate LS1
        ls2_trigger.write(false); // Deactivate LS2
        ls3_trigger.write(false); // Deactivate LS3
        ls4_trigger.write(false); // Deactivate LS4


        wait(SC_ZERO_TIME); // Allow LS1 to process

        ls1_done.notify();
        ls2_done.notify();
        ls3_done.notify();
        ls4_done.notify();

       
        wait(SC_ZERO_TIME);
        // Debugging: Print updated memeplexes after local search
        /*cout << "After Local Search - Memeplex 1:" << endl;
        for (int i = 0; i < 5; ++i) {
            Frog f = ls_output1[i].read();
            cout << "Position: ";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << ", Fitness: " << f.fitness << endl;
        }

        cout << "After Local Search - Memeplex 2:" << endl;
        for (int i = 0; i < 5; ++i) {
            Frog f = ls_output2[i].read();
            cout << "Position: ";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << ", Fitness: " << f.fitness << endl;
        }

        cout << "After Local Search - Memeplex 3:" << endl;
        for (int i = 0; i < 5; ++i) {
            Frog f = ls_output3[i].read();
            cout << "Position: ";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << ", Fitness: " << f.fitness << endl;
        }

        cout << "After Local Search - Memeplex 4:" << endl;
        for (int i = 0; i < 5; ++i) {
            Frog f = ls_output4[i].read();
            cout << "Position: ";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << ", Fitness: " << f.fitness << endl;
        }*/
    }

    void run_shuffler() {
        wait(ls1_done & ls2_done & ls3_done & ls4_done);
        shuffle_trigger.write(true); // Activate shuffler
        wait(SC_ZERO_TIME); // Allow shuffler to process
        shuffle_trigger.write(false); // Deactivate shuffler

        wait(SC_ZERO_TIME);
        // Debugging: Print shuffled frogs
        /*cout << "After shuffling:" << endl;
        for (int i = 0; i < 20; ++i) {
            Frog f = shuffled_frogs[i].read();
            cout << "Frog " << i << ": [";
            for (size_t j = 0; j < f.position.size(); ++j) {
                cout << f.position[j];
            }
            cout << "] Fitness: " << f.fitness << endl;
        }*/

        shuffler_done.notify();
    }
    // Function to save the shuffled frog population to a CSV file
    void saveShufflerOutput(const sc_vector<sc_signal<Frog>>& shuffledFrogs, const std::string& filename) {
        std::ofstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file " << filename << std::endl;
            return;
        }

        wait(SC_ZERO_TIME);
        for (const auto& frogSignal : shuffledFrogs) {
            Frog frog = frogSignal.read(); // Read the Frog signal

            // Write solution vector
            for (size_t i = 0; i < frog.position.size(); ++i) {
                file << frog.position[i];
                if (i < frog.position.size() - 1) file << ",";
            }

            // Write fitness value
            file << "," << frog.fitness << "\n";
        }

        file.close();
        std::cout << "Shuffled frog population saved to " << filename << std::endl;
    }
    

};

int sc_main(int argc, char* argv[]) {
    SFLA_System top("top");
    sc_start(10, SC_NS);

    // Print final results
    //cout << "Final frog population:" << endl;
    for (int i = 0; i < 20; ++i) {
        Frog f = top.shuffled_frogs[i].read();
        cout << "Frog " << i << ": [";
        for (size_t j = 0; j < f.position.size(); ++j) {
            cout << f.position[j];
        }
        cout << "] Fitness: " << f.fitness << endl;
    }
    return 0;
}