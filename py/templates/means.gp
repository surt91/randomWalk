{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set key bottom

f(x) = mu_c - a*x**(-b)
fit f(x) "{{ path }}/means.dat" u 1:2 via a, b, mu_c

p "{{ path }}/means.dat" u 1:2:3 w ye, \
  f(x) t sprintf("{/Symbol m} = {/Symbol m}_c - aT^{-b}, {/Symbol m}_c = %.3f(%.0f), {/Symbol c} = %.1f", mu_c, mu_c_err*1e3/FIT_STDFIT, FIT_STDFIT*FIT_STDFIT)

{% endblock content %}
