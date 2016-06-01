{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

set log y

plot \
{% for N in number_of_steps %}
    "{{ path }}/WL_{{ makebase(basename, steps=N) }}.dat" u ($1/{{ N }}**{{ dimension }}):(-$3/{{ N }}):(-$4/{{ N }}) w ye pt 1 t "{{ N }}", \
{% endfor %}

{% endblock content %}
