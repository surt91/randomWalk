{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set key bottom

d = {{ dimension }}
nu = (d-1)*{{ nu[typ][dimension] }}

f(x) = mu_c - a*x**(-b)
fit f(x) "{{ path }}/simple.dat" u 1:($22/$1**nu):($23/$1**nu) yerr via a, b, mu_c

p "{{ path }}/simple.dat" u 1:($22/$1**nu):($23/$1**nu) w ye t "{/Symbol m}", \
  f(x) t sprintf("{/Symbol m} = {/Symbol m}_c - aT^{-b}, {/Symbol m}_c = %.3f(%.0f), {/Symbol c} = %.1f", mu_c, mu_c_err*1e3/FIT_STDFIT, FIT_STDFIT*FIT_STDFIT)

{% endblock content %}
