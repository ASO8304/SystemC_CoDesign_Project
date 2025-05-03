clc; clear;

%% Add crucial directory validation and path handling
project_dir = 'F:\University\Term 5\CoDesign\Project\CoDesignProject\';
cd(project_dir); 

%% Constants and Parameters
num_frogs = 20;
num_items = 9;
max_capacity = 25;
max_no_change_iterations = 10;
max_iterations = 500;
tolerance = 1e-6; % Small tolerance for floating-point comparison
top_k = 10; % Number of top frogs to check for convergence

%% File Path Configuration
input_params_file = fullfile(project_dir, 'input_parameters.csv');
frog_pop_file = fullfile(project_dir, 'frog_population.csv');
program_exe = fullfile(project_dir, 'x64\Debug\CoDesignProject.exe');
output_file = fullfile(project_dir, 'final_result.csv');

%% Initialize Problem Data
prices = [6, 5, 8, 9, 6, 7, 3, 6, 8];
weights = [2, 3, 6, 7, 5, 9, 3, 4, 5];

input_data = [1:num_items; prices; weights]';
csvwrite(input_params_file, input_data);
fprintf('Input parameters written to: %s\n', input_params_file);

%% Initialize Frog Population
frogs = zeros(num_frogs, num_items + 1);
for i = 1:num_frogs
    position = randi([0, 1], 1, num_items);
    total_weight = sum(position .* weights);
    frogs(i, end) = sum(position .* prices) * (total_weight <= max_capacity);
    frogs(i, 1:num_items) = position;
end

sorted_frogs = sortrows(frogs, -size(frogs, 2));
csvwrite(frog_pop_file, sorted_frogs);
fprintf('Initial frog population saved to: %s\n', frog_pop_file);

%% Main Execution Loop with enhanced file handling
setenv('SC_SIGNAL_WRITE_CHECK', 'DISABLE');
last_top_k = [];
no_change_count = 0;
iteration = 0;

while (no_change_count < max_no_change_iterations) && (iteration < max_iterations)
    iteration = iteration + 1;
    fprintf('\n--- Iteration %d ---\n', iteration);
    
    % Verify file existence before execution
    if ~exist(frog_pop_file, 'file')
        error('Missing input file: %s', frog_pop_file);
    end
    
    % Execute with explicit working directory
    [status, cmdout] = system(['"', program_exe, '"'], '-echo');
    if status ~= 0
        error('SystemC execution failed:\n%s', cmdout);
    end
    
    % Read output with retry logic
    max_retries = 5;
    for retry = 1:max_retries
        try
            current_output = csvread(output_file);
            break;
        catch
            if retry == max_retries
                error('Failed to read output file after %d attempts', max_retries);
            end
            pause(0.1);
        end
    end

    % Extract top K frogs for convergence check
    current_top_k = current_output(1:top_k, :);

    % Check if the top K frogs have changed
    if ~isempty(last_top_k)
        % Allow a small numerical tolerance in comparisons
        if isequal(size(last_top_k), size(current_top_k)) && all(abs(last_top_k(:) - current_top_k(:)) < tolerance)
            no_change_count = no_change_count + 1;
            fprintf('Top %d frogs unchanged (%d/%d)\n', top_k, no_change_count, max_no_change_iterations);
        else
            no_change_count = 0;
            last_top_k = current_top_k;
            fprintf('New solution found in top %d frogs. Resetting counter.\n', top_k);
        end
    else
        no_change_count = 0;
        last_top_k = current_top_k;
    end

    % Save updated frog population
    if exist(frog_pop_file, 'file')
        prev_frogs = csvread(frog_pop_file);
        if isequal(prev_frogs, current_output)
            fprintf('No changes in frog population. Skipping overwrite.\n');
        else
            csvwrite(frog_pop_file, current_output);
            fprintf('Updated frog population saved for next iteration.\n');
        end
    else
        csvwrite(frog_pop_file, current_output);
        fprintf('Initial frog population file created.\n');
    end
end

% Print the best solution when iteration ends
best_frog = current_output(1, :);  % Frog 0 is the best solution
best_solution = best_frog(1:num_items);  % Extract the positions (binary selection)
best_fitness = best_frog(end);  % Extract the fitness

fprintf('\nOptimization completed successfully after %d iterations!\n', iteration);
fprintf('Selected Items: %s\n', mat2str(best_solution));
fprintf('Fitness: %.2f\n', best_fitness);
