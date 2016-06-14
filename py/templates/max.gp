{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set fit errorvariables
nu = {{ nu[typ][dimension] }}
d = {{ dimension }}
b1 = nu*d
b2 = nu*d
f1(x) = a1*x**b1
f2(x) = b2*log(x)+a2
fit f1(x) "{{ path }}/max.dat" u 1:2:3 yerr via a1, b1
chi1 = FIT_STDFIT
fit f2(x) "{{ path }}/max.dat" u 1:4:5 yerr via a2, b2
chi2 = FIT_STDFIT

plot "{{ path }}/max.dat" u 1:2:3 w yerr t "maxima", \
     f1(x) t sprintf("exponent: %.2f(%.0f), expected: %.2f, chi^2 = %.1f", b1, b1_err, nu*d, chi1**2)


{% endblock content %}
