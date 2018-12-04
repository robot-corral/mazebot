% arguments:
%   original_data_set_file_names - array of file names from which
%       normalization data is to be inferred
% returns:
%   [2, NUMBER_OF_SENSORS] array, 1st row contains min value (largest white
%   value), 2nd row contains max value (largest black value)
function min_max_data = compute_normalization_parameters(original_data_set_file_names)
    original_data = csvread(original_data_set_file_names(1));
    number_of_sensors = size(original_data, 2) - 1;
    min_max_data = zeros(2, number_of_sensors);
    for i = 1 : length(original_data_set_file_names)
        original_data = csvread(original_data_set_file_names(i));
        original_data = original_data(:, 2 : end);
        [~, first_row_max_index] = max(original_data(:, 1));
        [~, last_row_max_index] = max(original_data(:, end));
        if first_row_max_index > last_row_max_index
            original_data = flipud(original_data);
        end
        [black_data, white_data] = split_to_black_and_white(original_data);
        min_values = max(white_data);
        max_values = max(black_data);
        min_max_data(1, :) = max(min_values, min_max_data(1, :));
        min_max_data(2, :) = max(max_values, min_max_data(2, :));
    end
end
