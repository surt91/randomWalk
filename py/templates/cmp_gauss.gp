{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set xr [-5:5]

mu = 0
sigma = 1
f(x) = 1/sigma/sqrt(2*3.141592) * exp(-(x-mu)**2/2/sigma**2)

plot \
{% for N in number_of_steps %}
    "{{ path }}/normed_{{ makebase(basename, steps=N) }}.dat" u 1:3:2:4 w xye pt 1 t "{{ N }}", \
{% endfor %}
    f(x) t "Gaussian"

{% endblock content %}
