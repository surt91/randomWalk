{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

nu = {{ 1 if observable == 2 else 0.5 }}

plot \
{% for N in number_of_steps %}
    "{{ path }}/WL_m{{ sampling }}_t{{ typ }}_w{{ observable }}_d{{ dimension }}_N{{ N }}_n{{ iterations }}_x{{ seedMC }}_y{{ seedR }}_T0.00000.dat" u ($1/{{ N }}**nu):($2/{{ N }}**nu) w p t "{{ N }}", \
{% endfor %}

{% endblock content %}
