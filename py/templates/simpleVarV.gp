{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set key bottom

d = {{ dimension }}
nu = d*{{ nu[typ][dimension] }}

f(x) = mu_c - a*x**(-b)
fit f(x) "{{ path }}/simple.dat" u 1:($8/$1**(2*nu)):($9/$1**(2*nu)) yerr via a, b, mu_c

p "{{ path }}/simple.dat" u 1:($8/$1**(2*nu)):($9/$1**(2*nu)) w ye t "{/Symbol s}", \
  f(x) t sprintf("{/Symbol s} = {/Symbol s}_c - aT^{-b}, {/Symbol s}_c = %.3f(%.0f), {/Symbol c} = %.1f", mu_c, mu_c_err*1e3/FIT_STDFIT, FIT_STDFIT*FIT_STDFIT)

{% endblock content %}
