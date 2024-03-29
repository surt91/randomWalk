{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set fit errorvariables
# should be a linear, I guess
f(x) = a*x + c
fit f(x) "{{ path }}/simple.dat" u 1:4:5 yerr via a, c

plot "{{ path }}/simple.dat" u 1:4:5 w yerr t "r", \
     f(x) t sprintf("fit: r = %.2f*sqrt(N)+%.2f, chi^2 = %.1f", a, c, FIT_STDFIT*FIT_STDFIT)

{% endblock content %}
