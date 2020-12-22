% This matlab code applies the Savitzky–Golay filter using the sgolay
% function. Was used to verify the code produced in c++ using the Eigen
% library

y = importdata("sample_data.txt");
N = 4;
M = 65;
 
coeff = sgolay(N,M);

ycenter = conv(y,coeff((M+1)/2,:),'valid');

ybegin = coeff(end:-1:(M+3)/2,:) * y(M:-1:1);
yend = coeff((M-1)/2:-1:1,:) * y(end:-1:end-(M-1));

x = [1:1:2048];

y_smooth = [ybegin; ycenter; yend];
p = plot(x, y, x, y_smooth);
p(2).LineWidth = 3;
legend('Noisy Sinusoid','S-G smoothed sinusoid')
