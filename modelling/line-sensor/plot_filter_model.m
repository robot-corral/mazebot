%*******************************************************************************
% Copyright (C) 2018 Pavel Krupets                                             *
%*******************************************************************************

% plots all sensor parabolas and sensor centerlines
%
% arguments:
%   parabola_parameters - [NUMBER_OF_SENSORS, 3] array of parabola
%       coefficients a, b, c for each sensor. This describes parabola:
%       a * y ^ 2 + b * y + c
function plot_filter_model(parabola_parameters)
    for sensor_idx = 1 : size(parabola_parameters, 1)
        a = parabola_parameters(sensor_idx, 1);
        b = parabola_parameters(sensor_idx, 2);
        c = parabola_parameters(sensor_idx, 3);
        y_peak = - b / (2 * a);
        root = sqrt(b ^ 2 - 4 * a * c);
        y_1 = (-b - root) / (a * 2);
        y_2 = (-b + root) / (a * 2);
        y_min = min(y_1, y_2);
        y_max = max(y_1, y_2);
        y_subrange = linspace(y_min, y_max, 200);
        plot(y_subrange, a * y_subrange .^2 + b * y_subrange + c);
        plot([y_peak y_peak], [0, 1]);
        ylim([0 1]);
    end
end
