data_r1_1k0_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-1.0k-r2-220k-h-2mm.csv', 1);
data_r1_1k2_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-1.2k-r2-220k-h-2mm.csv', 1);
data_r1_1k5_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-1.5k-r2-220k-h-2mm.csv', 1);
data_r1_2k2_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-2.2k-r2-220k-h-2mm.csv', 1);
data_r1_2k7_r2_220k_h_2 = csvread('./data/sensor-output-voltage-and-current-consumption-r1-2.7k-r2-220k-h-2mm.csv', 1);

voltage_in_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 1);
all_black_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 2:4);
black_thin_line_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 5:7);
all_white_r1_1k0_r2_220k_h_2 = data_r1_1k0_r2_220k_h_2(:, 8:10);

voltage_in_r1_1k2_r2_220k_h_2 = data_r1_1k2_r2_220k_h_2(:, 1);
all_black_r1_1k2_r2_220k_h_2 = data_r1_1k2_r2_220k_h_2(:, 2:4);
black_thin_line_r1_1k2_r2_220k_h_2 = data_r1_1k2_r2_220k_h_2(:, 5:7);
all_white_r1_1k2_r2_220k_h_2 = data_r1_1k2_r2_220k_h_2(:, 8:10);

voltage_in_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 1);
all_black_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 2:4);
black_thin_line_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 5:7);
all_white_r1_1k5_r2_220k_h_2 = data_r1_1k5_r2_220k_h_2(:, 8:10);

voltage_in_r1_2k2_r2_220k_h_2 = data_r1_2k2_r2_220k_h_2(:, 1);
all_black_r1_2k2_r2_220k_h_2 = data_r1_2k2_r2_220k_h_2(:, 2:4);
black_thin_line_r1_2k2_r2_220k_h_2 = data_r1_2k2_r2_220k_h_2(:, 5:7);
all_white_r1_2k2_r2_220k_h_2 = data_r1_2k2_r2_220k_h_2(:, 8:10);

voltage_in_r1_2k7_r2_220k_h_2 = data_r1_2k7_r2_220k_h_2(:, 1);
all_black_r1_2k7_r2_220k_h_2 = data_r1_2k7_r2_220k_h_2(:, 2:4);
black_thin_line_r1_2k7_r2_220k_h_2 = data_r1_2k7_r2_220k_h_2(:, 5:7);
all_white_r1_2k7_r2_220k_h_2 = data_r1_2k7_r2_220k_h_2(:, 8:10);

% output voltage depenence on feature

figure(1);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])

hold on;
grid on;
xlim([2.5 4.7]);
plot(voltage_in_r1_1k5_r2_220k_h_2, all_black_r1_1k5_r2_220k_h_2(:, 2),'r-');
plot(voltage_in_r1_1k5_r2_220k_h_2, black_thin_line_r1_1k5_r2_220k_h_2(:, 2),'b-');
plot(voltage_in_r1_1k5_r2_220k_h_2, all_white_r1_1k5_r2_220k_h_2(:, 2),'k-');
xlabel('input voltage (V)');
ylabel('output voltage (V)');
legend('over all black', ...
       'over thin black line', ...
       'over all white');

hold off;

% output voltage depenence on input voltage

figure(2);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])

hold on;
grid on;
xlim([2.5 4.7]);
plot(voltage_in_r1_1k0_r2_220k_h_2, all_black_r1_1k0_r2_220k_h_2(:, 2),'r-');
plot(voltage_in_r1_1k0_r2_220k_h_2, black_thin_line_r1_1k0_r2_220k_h_2(:, 2),'b-');
plot(voltage_in_r1_1k0_r2_220k_h_2, all_white_r1_1k0_r2_220k_h_2(:, 2),'k-');
xlabel('input voltage (V)');
ylabel('output voltage (V)');
legend('over all black', ...
       'over thin black line', ...
       'over all white');

hold off;

% output voltage dependence on R1

resistors = [1000, 1200, 1500, 2200, 2700];
all_black = [all_black_r1_1k0_r2_220k_h_2(:, 2) ...
             all_black_r1_1k2_r2_220k_h_2(:, 2) ...
             all_black_r1_1k5_r2_220k_h_2(:, 2) ...
             all_black_r1_2k2_r2_220k_h_2(:, 2) ...
             all_black_r1_2k7_r2_220k_h_2(:, 2)];
all_white = [all_white_r1_1k0_r2_220k_h_2(:, 2) ...
             all_white_r1_1k2_r2_220k_h_2(:, 2) ...
             all_white_r1_1k5_r2_220k_h_2(:, 2) ...
             all_white_r1_2k2_r2_220k_h_2(:, 2) ...
             all_white_r1_2k7_r2_220k_h_2(:, 2)];
black_thin_line = [black_thin_line_r1_1k0_r2_220k_h_2(:, 2) ...
                   black_thin_line_r1_1k2_r2_220k_h_2(:, 2) ...
                   black_thin_line_r1_1k5_r2_220k_h_2(:, 2) ...
                   black_thin_line_r1_2k2_r2_220k_h_2(:, 2) ...
                   black_thin_line_r1_2k7_r2_220k_h_2(:, 2)];

figure(3);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])

hold on;
grid on;
xlim([1000 2700]);
plot(resistors, all_black(1, :),'r-', resistors, all_black(6, :),'r--', resistors, all_black(12, :),'r:');
plot(resistors, black_thin_line(1, :),'k-', resistors, black_thin_line(6, :),'k--', resistors, black_thin_line(12, :),'k:');
xlabel('R1 resistance (Ohm)');
ylabel('output voltage (V)');
legend('over all black (V_in = 2.5 V)', ...
       'over all black (V_in = 3.5 V)', ...
       'over all black (V_in = 4.7 V)', ...
       'over thin black line (V_in = 2.5 V)', ...
       'over thin black line (V_in = 3.5 V)', ...
       'over thin black line (V_in = 4.7 V)');

hold off;

figure(4);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])

hold on;
grid on;
xlim([1000 2700]);
plot(resistors, all_white(1, :),'b-', resistors, all_white(6, :),'b--', resistors, all_white(12, :),'b:');
xlabel('R1 resistance (Ohm)');
ylabel('output voltage (V)');
legend('over all white (V_in = 2.5 V)', ...
       'over all white (V_in = 3.5 V)', ...
       'over all white (V_in = 4.7 V)');

hold off;

% variant 1a design

figure(5);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])

hold on;
grid on;
xlim([2.5 4.7]);

xlabel('input voltage (V)');
ylabel('output voltage (V)');

% 1.0k

plot(voltage_in_r1_1k0_r2_220k_h_2, all_black_r1_1k0_r2_220k_h_2(:, 2),'r-', ...
     voltage_in_r1_1k0_r2_220k_h_2, all_black_r1_1k0_r2_220k_h_2(:, 3),'r:');

% 1.2k

plot(voltage_in_r1_1k2_r2_220k_h_2, all_black_r1_1k2_r2_220k_h_2(:, 2),'b-', ...
     voltage_in_r1_1k2_r2_220k_h_2, all_black_r1_1k2_r2_220k_h_2(:, 3),'b:');

% 1.5k

plot(voltage_in_r1_1k5_r2_220k_h_2, all_black_r1_1k5_r2_220k_h_2(:, 2),'k-', ...
     voltage_in_r1_1k5_r2_220k_h_2, all_black_r1_1k5_r2_220k_h_2(:, 3),'k:');

% 2.2k

plot(voltage_in_r1_2k2_r2_220k_h_2, all_black_r1_2k2_r2_220k_h_2(:, 2),'m-', ...
     voltage_in_r1_2k2_r2_220k_h_2, all_black_r1_2k2_r2_220k_h_2(:, 3),'m:');

% 2.2k

plot(voltage_in_r1_2k7_r2_220k_h_2, all_black_r1_2k7_r2_220k_h_2(:, 2),'g-', ...
     voltage_in_r1_2k7_r2_220k_h_2, all_black_r1_2k7_r2_220k_h_2(:, 3),'g:');

% uncomment for variant 1a
%plot([3.3, 3.3], [2, 4.5], 'k-')
% uncomment for variant 1b, 2a
plot([2.5 4.7], [3.3, 3.3], 'k-')
plot([2.5 4.7], [3.2, 3.2], 'k--')
plot([3.7 3.7], [2, 4.5], 'k--')

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

figure(6);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])

hold on;
grid on;
xlim([2.5 4.7]);

xlabel('input voltage (V)');
ylabel('output voltage resolution (V)');
ylabel('output voltage difference between all black / thin black line and all white (V)');

plot(voltage_in_r1_1k0_r2_220k_h_2, all_black_r1_1k0_r2_220k_h_2(:, 2) - all_white_r1_1k0_r2_220k_h_2(:, 2),'r-');
plot(voltage_in_r1_1k2_r2_220k_h_2, all_black_r1_1k2_r2_220k_h_2(:, 2) - all_white_r1_1k2_r2_220k_h_2(:, 2),'b-');
plot(voltage_in_r1_1k5_r2_220k_h_2, all_black_r1_1k5_r2_220k_h_2(:, 2) - all_white_r1_1k5_r2_220k_h_2(:, 2),'k-');
plot(voltage_in_r1_2k2_r2_220k_h_2, all_black_r1_2k2_r2_220k_h_2(:, 2) - all_white_r1_2k2_r2_220k_h_2(:, 2),'m-');
plot(voltage_in_r1_2k7_r2_220k_h_2, all_black_r1_2k7_r2_220k_h_2(:, 2) - all_white_r1_2k7_r2_220k_h_2(:, 2),'g-');

plot(voltage_in_r1_1k0_r2_220k_h_2, black_thin_line_r1_1k0_r2_220k_h_2(:, 2) - all_white_r1_1k0_r2_220k_h_2(:, 2),'r--');
plot(voltage_in_r1_1k2_r2_220k_h_2, black_thin_line_r1_1k2_r2_220k_h_2(:, 2) - all_white_r1_1k2_r2_220k_h_2(:, 2),'b--');
plot(voltage_in_r1_1k5_r2_220k_h_2, black_thin_line_r1_1k5_r2_220k_h_2(:, 2) - all_white_r1_1k5_r2_220k_h_2(:, 2),'k--');
plot(voltage_in_r1_2k2_r2_220k_h_2, black_thin_line_r1_2k2_r2_220k_h_2(:, 2) - all_white_r1_2k2_r2_220k_h_2(:, 2),'m--');
plot(voltage_in_r1_2k7_r2_220k_h_2, black_thin_line_r1_2k7_r2_220k_h_2(:, 2) - all_white_r1_2k7_r2_220k_h_2(:, 2),'g--');

% uncomment for variant 1a
%plot([3.3, 3.3], [0.5, 4], 'k-')
% uncomment for variant 1b, 2a
plot([3.7, 3.7], [0.5, 4], 'k-')

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

figure(7);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])

hold on;
grid on;
xlim([2.5 4.7]);

xlabel('input voltage (V)');
ylabel('output voltage difference between all black and thin black line (V)');

plot(voltage_in_r1_1k0_r2_220k_h_2, all_black_r1_1k0_r2_220k_h_2(:, 2) - black_thin_line_r1_1k0_r2_220k_h_2(:, 2),'r-');
plot(voltage_in_r1_1k2_r2_220k_h_2, all_black_r1_1k2_r2_220k_h_2(:, 2) - black_thin_line_r1_1k2_r2_220k_h_2(:, 2),'b-');
plot(voltage_in_r1_1k5_r2_220k_h_2, all_black_r1_1k5_r2_220k_h_2(:, 2) - black_thin_line_r1_1k5_r2_220k_h_2(:, 2),'k-');
plot(voltage_in_r1_2k2_r2_220k_h_2, all_black_r1_2k2_r2_220k_h_2(:, 2) - black_thin_line_r1_2k2_r2_220k_h_2(:, 2),'m-');
plot(voltage_in_r1_2k7_r2_220k_h_2, all_black_r1_2k7_r2_220k_h_2(:, 2) - black_thin_line_r1_2k7_r2_220k_h_2(:, 2),'g-');

plot([3.7, 3.7], [0, 1.8], 'k-')

legend('R1=1.0k {all black} - {black line}', ...
       'R1=1.2k {all black} - {black line}', ...
       'R1=1.5k {all black} - {black line}', ...
       'R1=2.2k {all black} - {black line}', ...
       'R1=2.7k {all black} - {black line}');

hold off;
