% function interpolates position for all the samples and calculates / fits
% parabolas for each sensor. Parabolas closely resemble what sensor sees
% when it moves across the black line (see documentation for more details).
%
% arguments:
%   normalized_data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] array which
%       contains sensor readings normalized so they all fall into [0 ... 1]
%       range (assumption is that 1st sensor crosses line 1st)
% returns:
%   y - [NUMBER_OF_SAMPLES] array of values of y estimated from the sensor
%       data or NaN if no sensor reads the line. See sensor configuration
%       documentation to get sensor displacement from the center.
%   parabola_parameters - [NUMBER_OF_SENSORS, 3] array of parabola
%       coefficients a, b, c for each sensor. This describes parabola:
%       a * y ^ 2 + b * y + c.
function [y, parabola_parameters] = compute_model_parameters(normalized_data)
    start_end_indexes = find_start_end(normalized_data);
    [min_y, peak_y, max_y] = fit_parabolas(normalized_data, start_end_indexes);
    y = interpolate_y(normalized_data, min_y, peak_y, max_y);
    parabola_parameters = calculate_parabola_parameters(normalized_data, y, min_y, peak_y, max_y);
end

% finds potential start and end indexes for values of a sensor where line
% has been sensed.
%
% arguments:
%   normalized_data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] sample data
%       for all sensors normalized so it fits into [0 ... 1] range
%       (assumption is that 1st sensor crosses line 1st)
% returns:
%   [NUMBER_OF_SENSORS, 2] array which contains start and end index for
%   each sensor
function [indexes] = find_start_end(normalized_data)
    number_of_sensors = size(normalized_data, 2);
    indexes = zeros(number_of_sensors, 2);
    for sensor_idx = 1 : number_of_sensors
        sensor_normalized_data = normalized_data(:, sensor_idx);
        start_idx = find(sensor_normalized_data ~= 0, 1, 'first');
        end_idx = find(sensor_normalized_data ~= 0, 1, 'last');
        number_of_samples = end_idx - start_idx + 1;
        slope = zeros(number_of_samples, 1);
        for sample_idx = 2 : number_of_samples
            slope(sample_idx) = sensor_normalized_data(start_idx + sample_idx - 1) - sensor_normalized_data(start_idx + sample_idx - 2);
        end
        % start and end of the parabola is determined by max and min of the
        % slope of sensed data
        y_start = start_idx + find(slope == max(slope), 1, 'first') - 1;
        y_end = start_idx + find(slope == min(slope), 1, 'last') - 1;
        indexes(sensor_idx, 1) = y_start;
        indexes(sensor_idx, 2) = y_end;
    end
end

% fits parabolas to the sensor data given start and end indexes of part of 
% data which represents sensed line (even partially).
%
% arguments:
%   normalized_data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] sample data
%       for all sensors normalized so it fits into [0 ... 1] range
%       (assumption is that 1st sensor crosses line 1st)
%   start_end_indexes - [NUMBER_OF_SENSORS, 2] array of start and end
%       indexes of data which represents sensor being over the line (even
%       partially)
%
% returns:
%   min_y - [NUMBER_OF_SENSORS] array of smallest values of y for sensor
%       value axis intercept
%   peak_y - [NUMBER_OF_SENSORS] array of values of y for maximum value of
%       the sensor reading
%   max_y - [NUMBER_OF_SENSORS] array of largest values of y for sensor
%       value axis intercept
function [min_y, peak_y, max_y] = fit_parabolas(normalized_data, start_end_indexes)
    number_of_samples = size(normalized_data, 1);
    number_of_sensors = size(normalized_data, 2);
    parabola_parameters = zeros(number_of_sensors, 5);

    for sensor_idx = 1 : number_of_sensors
        value_indexes = (start_end_indexes(sensor_idx, 1) : start_end_indexes(sensor_idx, 2))';
        [parabola_parameters(sensor_idx, 1 : 3), ~, parabola_parameters(sensor_idx, 4 : 5)] = ...
            polyfit(value_indexes, normalized_data(start_end_indexes(sensor_idx, 1) : start_end_indexes(sensor_idx, 2), sensor_idx), 2);
    end

    a = parabola_parameters(:, 1);
    b = parabola_parameters(:, 2);
    c = parabola_parameters(:, 3);
    % mu_1 and mu_2 are produced by polyfit rescaling and centering
    % parabolas for better fitting
    mu_1 = parabola_parameters(:, 4);
    mu_2 = parabola_parameters(:, 5);

    twice_a = 2 * a;
    numerator = twice_a .* mu_1 - b .* mu_2;
    root = mu_2 .* sqrt(b .^ 2 - 4 * a .* c);

    y_1 = (numerator - root) ./ twice_a;
    y_2 = (numerator + root) ./ twice_a;

    min_y = max(ones(number_of_sensors, 1), ceil(min(y_1, y_2)));
    peak_y = round(numerator ./ twice_a);
    max_y = min(ones(number_of_sensors, 1) * number_of_samples, floor(max(y_1, y_2)));

    % plot_data(normalized_data, min_y, max_y, a, b, c, mu_1, mu_2);
end

% function can be used to plit fitted parabolas and original data for
% debugging purposes.
%
% parabola is described by the following equation:
%   a * ((y - mu_1) / mu_2) ^ 2 + b * (y - mu_1) / mu_2 + c
%
% arguments:
%   normalized_data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] sample data
%       for all sensors normalized so it fits into [0 ... 1] range
%       (assumption is that 1st sensor crosses line 1st)
%   min_y - [NUMBER_OF_SENSORS] array of smallest values of y for sensor
%       value axis intercept
%   max_y - [NUMBER_OF_SENSORS] array of largest values of y for sensor
%       value axis intercept
%   a - [NUMBER_OF_SENSORS] array of parabola a coefficients
%   b - [NUMBER_OF_SENSORS] array of parabola b coefficients
%   c - [NUMBER_OF_SENSORS] array of parabola c coefficients
%   mu_1 - [NUMBER_OF_SENSORS] array of parabola centering coefficients
%   mu_2 - [NUMBER_OF_SENSORS] array of parabola scaling coefficients
function plot_data(normalized_data, min_y, max_y, a, b, c, mu_1, mu_2)
    number_of_sensors = size(normalized_data, 2);
    for sensor_idx = 1 : number_of_sensors
        smooth_y = linspace(min_y(sensor_idx), max_y(sensor_idx), 100);
        hold on;
        plot(smooth_y, a(sensor_idx) * ((smooth_y - mu_1(sensor_idx)) / mu_2(sensor_idx)) .^ 2 + b(sensor_idx) * (smooth_y - mu_1(sensor_idx)) / mu_2(sensor_idx) + c(sensor_idx));
        raw_y = min_y(sensor_idx) : max_y(sensor_idx);
        plot(raw_y, normalized_data(raw_y, sensor_idx));
        hold off;
    end
end

% function interpolates values of y of sensed line samples
%
% arguments:
%   normalized_data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] sample data
%       for all sensors normalized so it fits into [0 ... 1] range
%       (assumption is that 1st sensor crosses line 1st)
%   min_y - [NUMBER_OF_SENSORS] array of smallest values of y for sensor
%       value axis intercept
%   peak_y - [NUMBER_OF_SENSORS] array of values of y for maximum value of
%       the sensor reading
%   max_y - [NUMBER_OF_SENSORS] array of largest values of y for sensor
%       value axis intercept
%
% returns:
%   y - [NUMBER_OF_SAMPLES] array of y values for sensed line samples, in
%       case no line is sensed NaN value is provided instead.
function y = interpolate_y(normalized_data, min_y, peak_y, max_y)
    distance_between_sensors = 0.004;
    number_of_samples = size(normalized_data, 1);
    number_of_sensors = size(normalized_data, 2);
    y = zeros(number_of_samples, 1) / 0;

    for sensor_idx = 2 : number_of_sensors
        start_index = peak_y(sensor_idx - 1);
        end_index = peak_y(sensor_idx);
        number_of_samples = end_index - start_index + 1;
        dy = distance_between_sensors / (number_of_samples - 1);
        sensor_start_y = (sensor_idx - 12 - 1) * distance_between_sensors;
        y(start_index : end_index) = sensor_start_y + (0 : number_of_samples - 1)' .* dy;
    end

    [new_y, new_y_start_index, new_y_end_index] = fix_missing_y_values(y, min_y(1), peak_y(1), max_y(1), 1);
    y(new_y_start_index : new_y_end_index) = new_y;
    [new_y, new_y_start_index, new_y_end_index] = fix_missing_y_values(y, min_y(end), peak_y(end), max_y(end), 0);
    y(new_y_start_index : new_y_end_index) = new_y;
end

% fixes missing y values for the first and the last sensors (1st we only
% estimate distances between parabola peaks).
%
% arguments:
%   y - [NUMBER_OF_SAMPLES] array of currently available values for y
%   min_y - smallest values of y for sensor value axis intercept
%   peak_y - y for maximum value of the sensor reading
%   max_y - largest values of y for sensor value axis intercept
%   is_left_part - which part of the parabola is being fixed, to the left
%       or to the right of the peak
%
% returns:
%   result_y - array of [y_end_index - y_start_index + 1] missing y values
%   y_start_index -  start index from which missing y values are provided
%   y_end_index - end index until which missing y values are provided
function [result_y, y_start_index, y_end_index] = fix_missing_y_values(y, min_y, peak_y, max_y, is_left_part)
    if is_left_part
        y_start_index = min_y;
        y_end_index = peak_y;
        number_of_samples = y_end_index - y_start_index + 1;
        half_sensed_distance = abs(y(max_y) - y(peak_y));
        dy = half_sensed_distance / (number_of_samples - 1);
        result_y = flipud((y(peak_y) - (0 : number_of_samples - 1) .* dy)');
    else
        y_start_index = peak_y;
        y_end_index = max_y;
        number_of_samples = y_end_index - y_start_index + 1;
        half_sensed_distance = abs(y(peak_y) - y(min_y));
        dy = half_sensed_distance / (number_of_samples - 1);
        result_y = (y(peak_y) + (0 : number_of_samples - 1) .* dy)';
    end
end

% computes final parabola parameters for sensor parabolas (for interpolated
% y axis parameters.
%
% arguments:
%   normalized_data - [NUMBER_OF_SAMPLES, NUMBER_OF_SENSORS] sample data
%       for all sensors normalized so it fits into [0 ... 1] range
%       (assumption is that 1st sensor crosses line 1st)
%   y - [NUMBER_OF_SAMPLES] array of y values for sensed line samples, in
%       case no line is sensed NaN value is provided instead.
%   all_min_y - [NUMBER_OF_SENSORS] array of smallest values of y for
%       sensor value axis intercept
%   all_peak_y - [NUMBER_OF_SENSORS] array of values of y for maximum value
%       of the sensor reading
%   all_max_y - [NUMBER_OF_SENSORS] array of largest values of y for sensor
%       value axis intercept
%
% returns:
%   [NUMBER_OF_SENSORS, 3] array of parabola coefficients a, b, c for each
%   sensor. This describes parabola: a * y ^ 2 + b * y + c.
function parabola_parameters = calculate_parabola_parameters(normalized_data, y, all_min_y, all_peak_y, all_max_y)
    number_of_sensors = size(normalized_data, 2);
    parabola_parameters = zeros(number_of_sensors, 3);
    for sensor_idx = 1 : number_of_sensors
        peak_y = y(all_peak_y(sensor_idx));
        peak_f = normalized_data(all_peak_y(sensor_idx), sensor_idx);
        intercept_y = y(all_min_y(sensor_idx));
        intercept_f = normalized_data(all_min_y(sensor_idx), sensor_idx);
        denominator = (intercept_y - peak_y) ^ 2;
        a = (intercept_f - peak_f) / denominator;
        b = 2 * peak_y * (peak_f - intercept_f) / denominator;
        c = (intercept_y ^ 2 * peak_f - 2 * intercept_y * peak_f * peak_y + intercept_f * peak_y ^ 2) / denominator;
        parabola_parameters(sensor_idx, :) = [a b c];
    end
end
