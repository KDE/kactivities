
X = [1]

hold off;
plot(X, "x");
hold on;

x_spl = [1:l];
xf_spl = [1:.25:l];
plot(interp1(x_spl, Y0, xf_spl, "cubic"), "r", "linewidth", 3);
plot(interp1(x_spl, Y1, xf_spl, "cubic"), "g", "linewidth", 3);
plot(interp1(x_spl, Y2, xf_spl, "cubic"), "b", "linewidth", 3);
