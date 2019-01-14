% function does its best to split data into black and white data. Black
% is data where sensor reads the line (at or close enough to the line to
% sense it) and white is where sensor isn't nowhere near the line.
%
% arguments:
%   data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array which contains
%       sampled sensor values (assumption is that 1st sensor crosses line
%       1st)
% returns:
%   black_data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array whose values
%       are 0 for what is suspected to be white and non zero for what is
%       suspected to be black
%   white_data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array whose values
%       are NaN for what is suspected to be black and something other than
%       NaN for what is suspected to be white
function [black_data, white_data] = split_to_black_and_white(data)
    epsilon = 0.0001;
    cutoff_values = min(data) + (max(data) - min(data)) / 2;

    number_of_sensors = size(data, 2);
    number_of_samples = size(data, 1);
    above_cutoff_value_filter = data > cutoff_values;
    black_filter = above_cutoff_value_filter;

    for sensor_idx = 1 : number_of_sensors
        start_idx = find(above_cutoff_value_filter(:, sensor_idx) == 1, 1, 'first');
        end_idx = find(above_cutoff_value_filter(:, sensor_idx) == 1, 1, 'last');

        if sensor_idx > 1
            for sample_idx = start_idx - 1 : -1 : 1
                if data(sample_idx, sensor_idx) < epsilon
                    break;
                end
                if above_cutoff_value_filter(sample_idx, sensor_idx - 1) == 0
                    break;
                end
                if above_cutoff_value_filter(sample_idx, sensor_idx - 1) == 1
                    black_filter(sample_idx, sensor_idx) = 1;
                end
            end
        end

        if sensor_idx < number_of_sensors            
            for sample_idx = end_idx + 1 : number_of_samples
                if data(sample_idx, sensor_idx) < epsilon
                    break;
                end
                if above_cutoff_value_filter(sample_idx, sensor_idx + 1) == 0
                    break;
                end
                if above_cutoff_value_filter(sample_idx, sensor_idx + 1) == 1
                    black_filter(sample_idx, sensor_idx) = 1;
                end
            end
        end
    end

    black_filter = fix_column_filter(black_filter, data, 1);
    black_filter = fix_column_filter(black_filter, data, number_of_sensors);

    white_filter = ones(size(black_filter));
    white_filter(black_filter == 1) = NaN;

    black_data = data .* black_filter;
    white_data = data .* white_filter;
end

% internal function is intended to fix 1st and last columns as they don't
% have sensors on both sides and their black value cutoff will be too high
% at one end (start or end). Function simply uses max white from the other
% side to find proper start or end of the sensed line.
%
% E.g. invalid column might look like: [400; 900; 400; 300; 200], on one
% end min black value is 200 and on one it is 400. This function will fix
% start value in this case to include values which are >= 200. So potential
% fix in this case can be [220; 300; 400; 900; 400; 300; 200] (actual 
% values are taken from 'data' argument from appropriate rows).
%
% arguments:
%   black_filter - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array of black
%       filter values which needs fixing
%   data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array of sensor values
%       data (assumption is that 1st sensor crosses line 1st)
%   column_idx - column index which require fixing (should be 1st or last
%       colument in black_filter)
% returns:
%   [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array of fixed black filter
%   where all values in the column start and end at what is most likely is
%   a white value
function result = fix_column_filter(black_filter, data, column_idx)
    first_non_zero_idx = find(black_filter(:, column_idx) == 1, 1, 'first');
    last_non_zero_idx = find(black_filter(:, column_idx) == 1, 1, 'last');

    result = black_filter;

    if data(first_non_zero_idx, column_idx) > data(last_non_zero_idx, column_idx)
        for i = first_non_zero_idx - 1 : -1 : 1
            if data(i, column_idx) < data(last_non_zero_idx, column_idx)
                break;
            end
            result(i, column_idx) = 1;
        end
    else
        for i = last_non_zero_idx + 1 : size(data, 1)
            if data(i, column_idx) < data(first_non_zero_idx, column_idx)
                break;
            end
            result(i, column_idx) = 1;
        end
    end
end
