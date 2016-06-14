{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

d = {{ dimension }}
nu = {{ "d" if observable == 2 else "(d-1)" }}*{{ nu[typ][dimension] }}

plot \
{% for N in number_of_steps %}
    "{{ path }}/whole_{{ makebase(basename, steps=N) }}.dat" u ($1/{{ N }}**nu):($3+log({{ N }}**nu)):($2/{{ N }}**nu):($4) w xyerr t "{{ N }}", \
{% endfor %}

{% endblock content %}
