{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

nu = {{ 1 if observable == 2 else 0.5 }}

plot \
{% for N in number_of_steps %}
    "{{ path }}/WL_{{ makebase(basename, steps=N) }}.dat" u ($1/{{ N }}**nu):($2/{{ N }}**nu):($3/{{ N }}**nu) w ye pt 1 t "{{ N }}", \
{% endfor %}

{% endblock content %}
