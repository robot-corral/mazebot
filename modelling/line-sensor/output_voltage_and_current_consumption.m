data_r1_1k0_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-1.0k-r2-220k-h-2mm.csv', 1);
data_r1_1k5_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-1.5k-r2-220k-h-2mm.csv', 1);

voltage_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 1);
current_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 2);
all_black_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 3:5);
line_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 6:8);
all_white_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 9:11);

voltage_in_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 1);
all_black_in_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 3:5);
line_in_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 6:8);

figure(1);

hold on;

xlim([2.5 4.7]);

xlabel('input voltage (V)');

yyaxis right;
ylabel('current (mA)');
plot(voltage_in_r1_1k0_r2_220k_h_2, current_in_r1_1k0_r2_220k_h_2, 'b-');

yyaxis left;
ylabel('output voltage (V)');

plot(voltage_in_r1_1k0_r2_220k_h_2, all_black_in_r1_1k0_r2_220k_h_2(:, 1),'r:', ...
     voltage_in_r1_1k0_r2_220k_h_2, all_black_in_r1_1k0_r2_220k_h_2(:, 2),'r-', ...
     voltage_in_r1_1k0_r2_220k_h_2, all_black_in_r1_1k0_r2_220k_h_2(:, 3),'r:');

plot(voltage_in_r1_1k0_r2_220k_h_2, line_in_r1_1k0_r2_220k_h_2(:, 1),'k:', ...
     voltage_in_r1_1k0_r2_220k_h_2, line_in_r1_1k0_r2_220k_h_2(:, 2),'k-', ...
     voltage_in_r1_1k0_r2_220k_h_2, line_in_r1_1k0_r2_220k_h_2(:, 3),'k:');

%{
plot(voltage_in_r1_1k5_r2_220k_h_2, all_black_in_r1_1k5_r2_220k_h_2(:, 1),'r:', ...
     voltage_in_r1_1k5_r2_220k_h_2, all_black_in_r1_1k5_r2_220k_h_2(:, 2),'r-', ...
     voltage_in_r1_1k5_r2_220k_h_2, all_black_in_r1_1k5_r2_220k_h_2(:, 3),'r:');

plot(voltage_in_r1_1k5_r2_220k_h_2, line_in_r1_1k5_r2_220k_h_2(:, 1),'k:', ...
     voltage_in_r1_1k5_r2_220k_h_2, line_in_r1_1k5_r2_220k_h_2(:, 2),'k-', ...
     voltage_in_r1_1k5_r2_220k_h_2, line_in_r1_1k5_r2_220k_h_2(:, 3),'k:');
%}
 
[max_line_output_voltage, max_line_output_voltage_index] = max(line_in_r1_1k0_r2_220k_h_2(:, 2));
max_line_input_voltage = voltage_in_r1_1k0_r2_220k_h_2(max_line_output_voltage_index);
min_output_voltage = min([min(all_black_in_r1_1k0_r2_220k_h_2(:)), min(line_in_r1_1k0_r2_220k_h_2(:))]);
max_output_voltage = max([max(all_black_in_r1_1k0_r2_220k_h_2(:)), max(line_in_r1_1k0_r2_220k_h_2(:))]);
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
plot(voltage_in_r1_1k0_r2_220k_h_2, all_white_in_r1_1k0_r2_220k_h_2(:, 2),'k-');
hold off;
