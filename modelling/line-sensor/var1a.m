all_black_histogram = csvread('./data/var1a-all-black-histogram.csv');
all_white_histogram = csvread('./data/var1a-all-white-histogram.csv');
thin_black_line_histogram = csvread('./data/var1a-thin-black-line-histogram.csv');

figure(1);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])
hold on;
grid on;
bar(all_black_histogram(:, 1), all_black_histogram(:, 2));
xlabel('ADC value (16 bit)');
ylabel('Number of hits');
hold off;

figure(2);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])
hold on;
grid on;
bar(all_white_histogram(:, 1), all_white_histogram(:, 2));
xlabel('ADC value (16 bit)');
ylabel('Number of hits');
hold off;

figure(3);
set(gcf, 'OuterPosition',  [100, 100, 100 + 1150, 100 + 768])
hold on;
grid on;
bar(thin_black_line_histogram(:, 1), thin_black_line_histogram(:, 2));
xlabel('ADC value (16 bit)');
ylabel('Number of hits');
hold off;
