{% extends "base.gp" %}

{% block content %}

set print "peakFit.dat"
print "# T mu err sigma**2 err a err chi**2"

{% for N in number_of_steps %}
    {{ header(filename+N|string, xlabel, ylabel) }}

    N = {{ N }}
    d = {{ dimension }}
    nu = {{ "d" if observable == 2 else "(d-1)" }}*{{ nu[typ][dimension] }}

    x_min = {{ fit_xrange[N][0] }}
    x_max = {{ fit_xrange[N][1] }}

    fname = "{{ path }}/{{ "whole" if sampling == 1 or sampling == 4 else "WL" }}_{{ makebase(basename, steps=N) }}.dat"

    a = 1
    mu = (x_min**(1.0/d) + x_max**(1.0/d))/2
    sigma2 = 0.1
    f(x) = a * x**(1.0/d-1) / sigma2 * exp(-(x**(1.0/d) - mu)**2 / (2*sigma2**2))
    fit [x_min:x_max] f(x) fname u ($1/N**nu):(exp($3+log(N**nu))):($4*exp($3+log(N**nu))) yerr via a, mu, sigma2
    c = FIT_STDFIT

    set xr [x_min - 0.5:x_max + 0.5]

    plot fname u ($1/N**nu):(exp($3+log(N**nu))):($2/N**nu):(0) w xye pt 1 t "$T=".N."$", \
        f(x) lw 4 t sprintf("$\\mu = %.2f(%.0f), \\sigma^2 = %.2f(%.0f), \\chi^2 = %.1f$", mu, mu_err*1e2, sigma2, sigma2_err*1e2, c**2)

    print N, mu, mu_err/c, sigma2, sigma2_err/c, a, a_err/c, c**2

{% endfor %}

{{ header(filename+"_extrapolate"|string, "T", "\\mu") }}

unset xr

set key bottom

b = 2./{{ dimension }}
f(x) = mu_c - a*x**(-b)
fit f(x) "peakFit.dat" u 1:2:3 yerr via a, b, mu_c

p "peakFit.dat" u 1:2:3 w ye, \
  f(x) t sprintf("{/Symbol m} = {/Symbol m}_c - aT^{-b}, {/Symbol m}_c = %.3f(%.0f), {/Symbol c} = %.1f", mu_c, mu_c_err*1e3/FIT_STDFIT, FIT_STDFIT*FIT_STDFIT)

{% endblock content %}
