% sensor 1 is on the right side (y displacement is -0.044 m)
% sensor 23 is on the left side (y displacement is 0.044 m)

data_set_file_names = ["./data/capture-manual-0-degrees-1.csv";
                       "./data/capture-manual-0-degrees-2.csv";
                       "./data/capture-manual-0-degrees-reverse-1.csv";
                       "./data/capture-manual-0-degrees-reverse-2.csv"];

number_of_data_sets = length(data_set_file_names);

data_filter = designfilt('lowpassiir', ...
                         'FilterOrder', 4, ...
                         'PassbandFrequency', 40, ...
                         'PassbandRipple', 0.2, ...
                         'SampleRate', 1600);

min_max_data = compute_normalization_parameters(data_set_file_names);

for i = 1 : number_of_data_sets
    raw_data = csvread(data_set_file_names(i));
    original_data = normalize_data(raw_data, min_max_data);
    number_of_sensors = size(original_data, 2);

    % filter doesn't do anything about phase shift of the filter
    %filtered_data = filter(data_filter, original_data);
    % filtfilt accounts for phase shift of the filter
    filtered_data = filtfilt(data_filter, original_data);
    filtered_data(filtered_data < 0) = 0;
    filtered_data(filtered_data > 1) = 1;

    [black_filtered_data, white_filtered_data] = split_to_black_and_white(filtered_data);
    [y, filtered_data_parabolas_parameters] = compute_model_parameters(black_filtered_data);

    if ~exist('all_parabola_parameters', 'var')
        all_parabola_parameters = zeros(number_of_data_sets, number_of_sensors, 3);
    end
    all_parabola_parameters(i, :, :) = filtered_data_parabolas_parameters;
end

final_parabola_parameters = merge_parabola_parameters(all_parabola_parameters);

figure(1);
hold on;
plot_filter_model(final_parabola_parameters);
hold off;

% estimate position for the data captured by moving robot

raw_sensor_data = csvread("./data/robot-run-1.csv");
y = estimate_sensor_position(raw_sensor_data(:, 2 : end), data_filter, min_max_data, final_parabola_parameters);
figure(2);
plot(y);
