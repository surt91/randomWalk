{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set key bottom

d = {{ dimension }}
nu = {{ "d" if observable == 2 else "(d-1)" }}*{{ nu[typ][dimension] }}

f(x) = mu_c - a*x**(-b)
fit f(x) "{{ path }}/means.dat" u 1:($4/($1**nu)**2):($5/($1**nu)**2) yerr via a, b, mu_c

p "{{ path }}/means.dat" u 1:($4/($1**nu)**2):($5/($1**nu)**2) w ye t "{/Symbol s}^2", \
  f(x) t sprintf("{/Symbol s}^2 = {/Symbol s}_c^2 - aT^{-b}, {/Symbol s}_c^2 = %.3f(%.0f), {/Symbol c} = %.1f", mu_c, mu_c_err*1e3/FIT_STDFIT, FIT_STDFIT*FIT_STDFIT)

{% endblock content %}
