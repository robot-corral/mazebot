data = csvread('./data/output-voltage-dependence-on-r1-r2-tolerance-vin3.5.csv', 1);
base_row = find(data(:, 1) == 1000 & data(:, 2) == 220000);
normalized_data = abs(1 - data ./ data(base_row, :));
disp("result in absolute % difference");
result = [data(:, 1:2), normalized_data(:, 3:4) * 100];
format short g;
disp(result);
