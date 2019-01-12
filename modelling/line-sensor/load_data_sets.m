% loads data for multiple data sets and applies filter to it. method also
% fixes data by ignoring step response data.
%
% all data sets must have the same number of sensors.
%
% arguments:
%   data_sets_file_names - [NUMBER_OF_DATA_SETS] data set file names
%
% returns:
%   number_of_sensors - (scalar) number of sensors in data sets.
%   data_sets - [NUMBER_OF_DATA_SETS, NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS]
%       loaded and fileted data sets. please note that they can have
%       different number of samples.
%   data_filter - data filter used to filter data sets.
%   settling_samples_count - settling time of a filter for step response.
function [number_of_sensors, data_sets, data_filter, settling_samples_count] = load_data_sets(data_sets_file_names)
    number_of_data_sets = length(data_sets_file_names);

    data_filter = designfilt('lowpassiir', ...
                             'FilterOrder', 4, ...
                             'PassbandFrequency', 40, ...
                             'PassbandRipple', 0.2, ...
                             'SampleRate', 1600);

    step_info = stepinfo(stepz(data_filter, 10 * 1600));
    settling_samples_count = ceil(step_info.SettlingTime);

    number_of_sensors = 0;
    data_sets = cell(number_of_data_sets);
    for i = 1 : number_of_data_sets
        raw_data = csvread(data_sets_file_names(i));
        raw_sensors_data = raw_data(:, 2 : end);
        if number_of_sensors == 0
            number_of_sensors = size(raw_sensors_data, 2);
        elseif number_of_sensors ~= size(raw_sensors_data, 2)
            error('all data sets must have the same number of sensors');
        end
        prefixed_raw_sensors_data = [repmat(raw_sensors_data(1, :), settling_samples_count, 1); raw_sensors_data];
        filtered_sensors_data = filtfilt(data_filter, prefixed_raw_sensors_data);
        stable_filtered_sensors_data = filtered_sensors_data(settling_samples_count + 1 : end, :);
        data_sets{i} = stable_filtered_sensors_data;
    end
end
