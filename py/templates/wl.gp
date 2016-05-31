{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

plot \
{% for N in number_of_steps %}
    "{{ path }}/WL_{{ makebase(basename, steps=N) }}.dat" u 1:3:2:4 w xye pt 1 t "{{ N }}", \
{% endfor %}

{% endblock content %}
