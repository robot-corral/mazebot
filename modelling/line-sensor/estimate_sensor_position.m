%*******************************************************************************
% Copyright (C) 2018 Pavel Krupets                                             *
%*******************************************************************************

% model is designed to follow a line which robot is supposed to follow. If
% robot crosses parallel lines special code will be used to handle this. In
% this case multiple sensors can be at maximum or at minimum or anywhere in
% between.
%
% arguments:
%   sensor_data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array which contains sensor readings
%   sensor_data_filter - filter to be applied to sensor_data (after it is normalized)
%   settling_samples_count - settling time of a filter for step response
%   sensor_normalization_parameters - [2, NUMBER_OF_SENSORS] array which contains min and max values for each sensor
%   sensor_model_parameters - [NUMBER_OF_SENSORS, 3] array of parabola coefficients a, b, c for each sensor. This
%       describes parabola: a * y ^ 2 + b * y + c.
%
% returns:
%   [NUMBER_OF_SAMPLES] array of estimated sensor position, if sensor position cannot be estimated NaN is used.
function y = estimate_sensor_position(sensor_data, sensor_data_filter, settling_samples_count, sensor_normalization_parameters, sensor_model_parameters)
    number_of_samples = size(sensor_data, 1);
    number_of_sensors = size(sensor_data, 2);

    prefixed_sensor_data = [repmat(sensor_data(1, :), settling_samples_count, 1); sensor_data];
    filtered_sensor_data = filtfilt(sensor_data_filter, prefixed_sensor_data);
    stable_filtered_sensor_data = filtered_sensor_data(settling_samples_count + 1 : end, :);
    stable_filtered_sensor_data(stable_filtered_sensor_data < 0) = 0;

    normalized_data = (stable_filtered_sensor_data - sensor_normalization_parameters(1, :)) ./ (sensor_normalization_parameters(2, :) - sensor_normalization_parameters(1, :));
    normalized_data(normalized_data < 0) = 0;
    normalized_data(normalized_data > 1) = 1;

    a = sensor_model_parameters(:, 1)';
    b = sensor_model_parameters(:, 2)';
    c = sensor_model_parameters(:, 3)';

    a_twice = 2 * a;
    a_for_times = 4 * a;
    b_squared = b .^ 2;

    max_f = c - b_squared ./ a_for_times;
    max_y = - b ./ a_twice;

    for sensor_index = 1 : number_of_sensors
        normalized_data(normalized_data(:, sensor_index) > max_f(sensor_index), sensor_index) = max_f(sensor_index);
    end

    black_filter = abs(normalized_data) > 0.05;

    root = sqrt(b_squared + a_for_times .* (normalized_data - c));
    y_1 = (-b - root) ./ (a_twice);
    y_2 = (-b + root) ./ (a_twice);
    y_1 = y_1 .* black_filter ./ black_filter;
    y_2 = y_2 .* black_filter ./ black_filter;

    y = zeros(number_of_samples, 1) / 0;

    for i = 1 : number_of_samples
        end_sensor_index = 0;
        start_sensor_index = 0;
        has_multiple_line_sensings = 0;
        for sensor_index = 1 : number_of_sensors
            if ~isnan(y_1(i, sensor_index)) || ~isnan(y_2(i, sensor_index))
                if start_sensor_index == 0
                    start_sensor_index = sensor_index;
                elseif end_sensor_index ~= 0
                    has_multiple_line_sensings = 1;
                end
            elseif start_sensor_index ~= 0 && end_sensor_index == 0 && isnan(y_1(i, sensor_index)) && isnan(y_2(i, sensor_index))
                end_sensor_index = sensor_index - 1;
            end
        end
        if start_sensor_index ~= 0 && end_sensor_index == 0
            end_sensor_index = number_of_sensors;
        end
        
        if start_sensor_index ~= 0 && has_multiple_line_sensings == 0
            number_of_active_sensors = end_sensor_index - start_sensor_index + 1;
            if number_of_active_sensors == 1
                if start_sensor_index == 1
                    y(i) = min(y_1(i, start_sensor_index), y_2(i, start_sensor_index));
                elseif start_sensor_index == number_of_sensors
                    y(i) = max(y_1(i, start_sensor_index), y_2(i, start_sensor_index));
                else
                    % not much can be done here (apart from bringing sensors closer together), hard to decide if y_1 or
                    % y_2 should be used. This results in horizontal lines in y where value cannot be interpolated
                    % without knowing future data.
                    y(i) = max_y(start_sensor_index);
                end
            elseif number_of_active_sensors > 1
                y_candidate_1 = max(y_1(i, start_sensor_index), y_2(i, start_sensor_index));
                % middle sensors are assumed to be fully over the line
                y_candidate_2 = min(y_1(i, end_sensor_index), y_2(i, end_sensor_index));
                y(i) = mean([y_candidate_1, y_candidate_2]);
            end
        end
    end
end
