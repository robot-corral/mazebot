%*******************************************************************************
% Copyright (C) 2018 Pavel Krupets                                             *
%*******************************************************************************

% arguments:
%   original_data_set_file_names - array of file names from which
%       normalization data is to be inferred
% returns:
%   [2, NUMBER_OF_SENSORS] array, 1st row contains min value (largest white
%   value), 2nd row contains max value (largest black value)
function min_max_data = compute_normalization_parameters(number_of_sensors, data_sets)
    min_max_data = zeros(2, number_of_sensors);
    for i = 1 : length(data_sets)
        data_set = data_sets{i};
        [~, first_row_max_index] = max(data_set(:, 1));
        [~, last_row_max_index] = max(data_set(:, end));
        if first_row_max_index > last_row_max_index
            data_set = flipud(data_set);
        end
        [black_data, white_data] = split_to_black_and_white(data_set);
        min_values = max(white_data);
        max_values = max(black_data);
        min_max_data(1, :) = max(min_values, min_max_data(1, :));
        min_max_data(2, :) = max(max_values, min_max_data(2, :));
    end
end
