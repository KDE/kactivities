
l = length(Y0);
jump = floor(l/7);
X = ones(1, l * 4) * 0;

for i = [1:jump*4:l*4],
    X(i) = 4;
end

hold off;
plot(X, "x");
hold on;

x_spl = [1:l];
xf_spl = [1:.25:l];
plot(interp1(x_spl, Y0, xf_spl, "cubic"), "r");
plot(interp1(x_spl, Y1, xf_spl, "cubic"), "g");
plot(interp1(x_spl, Y2, xf_spl, "cubic"), "b");
