data = csvread('./data/sensor-output-voltage-and-current-consumption.csv', 1);
voltage_in = data(:, 1);
current = data(:, 2);
all_black = data(:, 3:5);
line = data(:, 6:8);
all_white = data(:, 9:11);
hold on;
figure(1);
xlabel('input voltage (V)');
yyaxis right;
ylabel('current (mA)');
plot(voltage_in, current, 'b-');
yyaxis left;
ylabel('output voltage (V)');
plot(voltage_in, all_black(:, 1),'r:', ...
     voltage_in, all_black(:, 2),'r-', ...
     voltage_in, all_black(:, 3),'r:');
plot(voltage_in, line(:, 1),'k:', ...
     voltage_in, line(:, 2),'k-', ...
     voltage_in, line(:, 3),'k:');
[max_line_output_voltage, max_line_output_voltage_index] = max(line(:, 2));
max_line_input_voltage = voltage_in(max_line_output_voltage_index);
min_output_voltage = min([min(all_black(:)), min(line(:))]);
max_output_voltage = max([max(all_black(:)), max(line(:))]);
plot([max_line_input_voltage, max_line_input_voltage], [min_output_voltage, max_output_voltage], 'm');
legend('all black V min', ...
       'all black V mean', ...
       'all black V max', ...
       'black line V min', ...
       'black line V mean', ...
       'black line V max', ...
       'max(black line V mean)', ...
       'current consumption');
hold off;

figure(2);
hold on;
xlabel('input voltage (V)');
ylabel('output voltage (V)');
plot(voltage_in, all_white(:, 2),'k-');
hold off;