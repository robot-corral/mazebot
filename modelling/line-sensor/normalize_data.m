% normalize data so it fits [0 ... 1] range and makes sure that data for
% the 1st sensor appears 1st.
%
% arguments:
%   data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array which contains
%       sensor readings
%   min_max_data - [2, NUMBER_OF_SENSORS] array, 1st row contains min value
%       (largest white value), 2nd row contains max value (largest black
%       value)
% returns:
%   [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array which contains sensor
%   readings normalized so they all fall into [0 ... 1] range. In case last
%   sensor comes 1st data will be flipped so 1st sensor data comes 1st.
function normalized_data = normalize_data(data, min_max_data)
    data = data(:, 2 : end);
    [~, first_row_max_index] = max(data(:, 1));
    [~, last_row_max_index] = max(data(:, end));
    normalized_data = (data - min_max_data(1, :)) ./ (min_max_data(2, :) - min_max_data(1, :));
    normalized_data(normalized_data < 0) = 0;
    normalized_data(normalized_data > 1) = 1;
    if first_row_max_index > last_row_max_index
        normalized_data = flipud(normalized_data);
    end
end
