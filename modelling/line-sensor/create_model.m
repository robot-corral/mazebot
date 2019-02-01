%*******************************************************************************
% Copyright (C) 2018 Pavel Krupets                                             *
%*******************************************************************************

% sensor 1 is on the right side (y displacement is -0.044 m)
% sensor 23 is on the left side (y displacement is 0.044 m)

data_set_file_names = ["./data/capture-manual-0-degrees-1.csv";
                       "./data/capture-manual-0-degrees-2.csv";
                       "./data/capture-manual-0-degrees-reverse-1.csv";
                       "./data/capture-manual-0-degrees-reverse-2.csv"];

[number_of_sensors, data_sets, data_filter, settling_samples_count] = load_data_sets(data_set_file_names);

min_max_data = compute_normalization_parameters(number_of_sensors, data_sets);

all_parabola_parameters = compute_model_parameters(number_of_sensors, min_max_data, data_sets);

final_parabola_parameters = merge_parabola_parameters(all_parabola_parameters);

% plot line sensor model

figure(2);
hold on;
plot_filter_model(final_parabola_parameters);
hold off;

% estimate position for the data captured by moving robot

raw_sensor_data = csvread("./data/robot-run-1.csv");
y = estimate_sensor_position(raw_sensor_data(:, 2 : end), data_filter, settling_samples_count, min_max_data, final_parabola_parameters);
figure(3);
plot(y);
