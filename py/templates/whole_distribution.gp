{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

plot \
{% for N in number_of_steps %}
    "{{ path }}/whole_m{{ sampling }}_t{{ typ }}_w{{ observable }}_d{{ dimension }}_N{{ N }}_n{{ iterations }}_x{{ seedMC }}_y{{ seedR }}.dat" u 1:3:2:4 w xyerr t "{{ N }}", \
{% endfor %}

{% endblock content %}
