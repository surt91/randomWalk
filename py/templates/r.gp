{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set fit errorvariables
# should be a sqrt, I guess
f(x) = a*x**0.5 + c
fit f(x) "{{ path }}/simple.dat" u 1:2:3 yerr via a, c

plot "{{ path }}/simple.dat" u 1:2:3 w yerr t "r", \
     f(x) t sprintf("fit: r = %.2f*sqrt(N)+%.2f, chi^2 = %.1f", a, c, FIT_STDFIT*FIT_STDFIT)

{% endblock content %}
