{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set log xy

d = {{ dimension }}.0

smax(x) = {{ getMaximumSForGnuplot(dimension, observable, typ) }}

plot \
{% for N in number_of_steps %}
    "{{ path }}/whole_{{ makebase(basename, steps=N) }}.dat" u ($1/smax({{ N }})):(-$3/{{ N }}):(-$4/{{ N }}) w ye pt 1 t "{{ N }}", \
{% endfor %}
    "phi_inf.dat" u 1:2:3 w ye pt 8 lc 8 t 'Asymptotic {/Symbol F}', \
{% endblock content %}
