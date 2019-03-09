data_r1_1k0_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-1.0k-r2-220k-h-2mm.csv', 1);
data_r1_1k2_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-1.2k-r2-220k-h-2mm.csv', 1);
data_r1_1k5_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-1.5k-r2-220k-h-2mm.csv', 1);
data_r1_2k2_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-2.2k-r2-220k-h-2mm.csv', 1);
data_r1_2k7_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-2.7k-r2-220k-h-2mm.csv', 1);

voltage_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 1);
all_black_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 2:4);
line_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 5:7);
all_white_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 8:10);

voltage_in_r1_1k2_r2_220k_h_2 = data_r1_1k2_r2_220k_h_2(:, 1);
all_black_in_r1_1k2_r2_220k_h_2 = data_r1_1k2_r2_220k_h_2(:, 2:4);
line_in_r1_1k2_r2_220k_h_2 = data_r1_1k2_r2_220k_h_2(:, 5:7);
all_white_in_r1_1k2_r2_220k_h_2 = data_r1_1k2_r2_220k_h_2(:, 8:10);

voltage_in_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 1);
all_black_in_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 2:4);
line_in_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 5:7);
all_white_in_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 8:10);

voltage_in_r1_2k2_r2_220k_h_2 = data_r1_2k2_r2_220k_h_2(:, 1);
all_black_in_r1_2k2_r2_220k_h_2 = data_r1_2k2_r2_220k_h_2(:, 2:4);
line_in_r1_2k2_r2_220k_h_2 = data_r1_2k2_r2_220k_h_2(:, 5:7);
all_white_in_r1_2k2_r2_220k_h_2 = data_r1_2k2_r2_220k_h_2(:, 8:10);

voltage_in_r1_2k7_r2_220k_h_2 = data_r1_2k7_r2_220k_h_2(:, 1);
all_black_in_r1_2k7_r2_220k_h_2 = data_r1_2k7_r2_220k_h_2(:, 2:4);
line_in_r1_2k7_r2_220k_h_2 = data_r1_2k7_r2_220k_h_2(:, 5:7);
all_white_in_r1_2k7_r2_220k_h_2 = data_r1_2k7_r2_220k_h_2(:, 8:10);

figure(1);

hold on;

xlim([2.5 4.7]);

xlabel('input voltage (V)');
ylabel('output voltage (V)');

% 1.0k

plot(voltage_in_r1_1k0_r2_220k_h_2, all_black_in_r1_1k0_r2_220k_h_2(:, 2),'r-', ...
     voltage_in_r1_1k0_r2_220k_h_2, all_black_in_r1_1k0_r2_220k_h_2(:, 3),'r:');

% 1.2k

plot(voltage_in_r1_1k2_r2_220k_h_2, all_black_in_r1_1k2_r2_220k_h_2(:, 2),'b-', ...
     voltage_in_r1_1k2_r2_220k_h_2, all_black_in_r1_1k2_r2_220k_h_2(:, 3),'b:');

% 1.5k

plot(voltage_in_r1_1k5_r2_220k_h_2, all_black_in_r1_1k5_r2_220k_h_2(:, 2),'k-', ...
     voltage_in_r1_1k5_r2_220k_h_2, all_black_in_r1_1k5_r2_220k_h_2(:, 3),'k:');

% 2.2k

plot(voltage_in_r1_2k2_r2_220k_h_2, all_black_in_r1_2k2_r2_220k_h_2(:, 2),'m-', ...
     voltage_in_r1_2k2_r2_220k_h_2, all_black_in_r1_2k2_r2_220k_h_2(:, 3),'m:');

% 2.2k

plot(voltage_in_r1_2k7_r2_220k_h_2, all_black_in_r1_2k7_r2_220k_h_2(:, 2),'g-', ...
     voltage_in_r1_2k7_r2_220k_h_2, all_black_in_r1_2k7_r2_220k_h_2(:, 3),'g:');

legend('R1=1.0k all black V mean', ...
       'R1=1.0k all black V max', ...
       'R1=1.2k all black V mean', ...
       'R1=1.2k all black V max', ...
       'R1=1.5k all black V mean', ...
       'R1=1.5k all black V max', ...
       'R1=2.2k all black V mean', ...
       'R1=2.2k all black V max', ...
       'R1=2.7k all black V mean', ...
       'R1=2.7k all black V max');

hold off;

figure(2);

hold on;

xlim([2.5 4.7]);

xlabel('input voltage (V)');
ylabel('output voltage (V)');

plot(voltage_in_r1_1k0_r2_220k_h_2, all_black_in_r1_1k0_r2_220k_h_2(:, 2) - all_white_in_r1_1k0_r2_220k_h_2(:, 2),'r-');
plot(voltage_in_r1_1k2_r2_220k_h_2, all_black_in_r1_1k2_r2_220k_h_2(:, 2) - all_white_in_r1_1k2_r2_220k_h_2(:, 2),'b-');
plot(voltage_in_r1_1k5_r2_220k_h_2, all_black_in_r1_1k5_r2_220k_h_2(:, 2) - all_white_in_r1_1k5_r2_220k_h_2(:, 2),'k-');
plot(voltage_in_r1_2k2_r2_220k_h_2, all_black_in_r1_2k2_r2_220k_h_2(:, 2) - all_white_in_r1_2k2_r2_220k_h_2(:, 2),'m-');
plot(voltage_in_r1_2k7_r2_220k_h_2, all_black_in_r1_2k7_r2_220k_h_2(:, 2) - all_white_in_r1_2k7_r2_220k_h_2(:, 2),'g-');

plot(voltage_in_r1_1k0_r2_220k_h_2, line_in_r1_1k0_r2_220k_h_2(:, 2) - all_white_in_r1_1k0_r2_220k_h_2(:, 2),'r--');
plot(voltage_in_r1_1k2_r2_220k_h_2, line_in_r1_1k2_r2_220k_h_2(:, 2) - all_white_in_r1_1k2_r2_220k_h_2(:, 2),'b--');
plot(voltage_in_r1_1k5_r2_220k_h_2, line_in_r1_1k5_r2_220k_h_2(:, 2) - all_white_in_r1_1k5_r2_220k_h_2(:, 2),'k--');
plot(voltage_in_r1_2k2_r2_220k_h_2, line_in_r1_2k2_r2_220k_h_2(:, 2) - all_white_in_r1_2k2_r2_220k_h_2(:, 2),'m--');
plot(voltage_in_r1_2k7_r2_220k_h_2, line_in_r1_2k7_r2_220k_h_2(:, 2) - all_white_in_r1_2k7_r2_220k_h_2(:, 2),'g--');

legend('R1=1.0k {all black} - {all white}', ...
       'R1=1.2k {all black} - {all white}', ...
       'R1=1.5k {all black} - {all white}', ...
       'R1=2.2k {all black} - {all white}', ...
       'R1=2.7k {all black} - {all white}', ...
       'R1=1.0k {black line} - {all white}', ...
       'R1=1.2k {black line} - {all white}', ...
       'R1=1.5k {black line} - {all white}', ...
       'R1=2.2k {black line} - {all white}', ...
       'R1=2.7k {black line} - {all white}');

hold off;
