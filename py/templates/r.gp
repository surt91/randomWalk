{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set fit errorvariables
nu = {{ nu[typ][dimension] }}
f(x) = a*x**b
fit f(x) "{{ path }}/simple.dat" u 1:2:3 yerr via a, b

plot "{{ path }}/simple.dat" u 1:2:3 w yerr t "r", \
     f(x) t sprintf("exponent: %.2f(%.0f), expected: %.2f, chi^2 = %.1f", b, b_err, nu, FIT_STDFIT**2)


{% endblock content %}
