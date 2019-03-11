data = csvread('./data/output-voltage-dependence-on-r1.csv', 1);
r1 = data(:, 1);
r2 = 220000;
v_in_3p5_to_v_out = data(:, 2:3);
v_in_3p0_to_v_out = data(:, 4:5);
v_in_2p5_to_v_out = data(:, 6:7);

figure(1);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])
hold on;
grid on;
xlabel('R1 (Ohm)');
ylabel('Output voltage (V)');
plot(r1, v_in_3p5_to_v_out(:, 1), 'b-', r1, v_in_3p5_to_v_out(:, 2), 'b--');
plot(r1, v_in_3p0_to_v_out(:, 1), 'r-', r1, v_in_3p0_to_v_out(:, 2), 'r--');
plot(r1, v_in_2p5_to_v_out(:, 1), 'k-', r1, v_in_2p5_to_v_out(:, 2), 'k--');
legend('sensor is over all white (V_in = 3.5 V)', ...
       'sensor is over all black (V_in = 3.5 V)', ...
       'sensor is over all white (V_in = 3 V)', ...
       'sensor is over all black (V_in = 3 V)', ...
       'sensor is over all white (V_in = 2.5 V)', ...
       'sensor is over all black (V_in = 2.5 V)');
hold off;
