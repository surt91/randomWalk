{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

plot \
{% for N in number_of_steps %}
    "{{ path }}/WL_m{{ sampling }}_t{{ typ }}_w{{ observable }}_d{{ dimension }}_N{{ N }}_n{{ iterations }}_x{{ seedMC }}_y{{ seedR }}.dat" u 1:2 w p t "{{ N }}", \
{% endfor %}

{% endblock content %}
