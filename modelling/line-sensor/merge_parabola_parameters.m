% given set of sensor parabolas finds 'average' one to be used as a final
% model
%
% arguments:
%   multiple_parabola_parameters - [NUMBER_OF_SETS, NUMBER_OF_SENSORS, 3]
%       array of data sets of parabola parameters for each sensor to be
%       used in consructing final model
%
% returns:
%   [NUMBER_OF_SENSORS, 3] array of parabola coefficients a, b, c for each
%   sensor. This describes parabola: a * y ^ 2 + b * y + c
function result_parabola_parameters = merge_parabola_parameters(multiple_parabola_parameters)
    a = multiple_parabola_parameters(:, :, 1);
    b = multiple_parabola_parameters(:, :, 2);
    c = multiple_parabola_parameters(:, :, 3);
    a_twice = 2 * a;
    root = sqrt(b .^ 2 - 4 * a .* c);
    y_intercept = (- b + root) ./ (a_twice);
    y_peak = - b ./ a_twice;
    f_peak = a .* y_peak .^ 2 + b .* y_peak + c;

    mean_y_intercept = mean(y_intercept);
    % y_peak is the same and is determined by the sensor physical location
    mean_y_peak = y_peak(1, :, :);
    mean_f_peak = max(f_peak);

    mean_denominator = (mean_y_intercept - mean_y_peak) .^ 2;
    mean_a = - mean_f_peak ./ mean_denominator;
    mean_b = 2 * mean_f_peak .* mean_y_peak ./ mean_denominator;
    mean_c = mean_y_intercept .* mean_f_peak .* (mean_y_intercept - 2 * mean_y_peak) ./ mean_denominator;

    result_parabola_parameters = [mean_a; mean_b; mean_c]';
end
