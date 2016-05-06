{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

nu = {{ 1 if observable == 2 else 0.5 }}

plot \
{% for N in number_of_steps %}
    "{{ path }}/whole_m{{ sampling }}_t{{ typ }}_w{{ observable }}_d{{ dimension }}_N{{ N }}_n{{ iterations }}_x{{ seedMC }}_y{{ seedR }}.dat" u ($1/{{ N }}**nu):($3+log({{ N }}**nu)):($2/{{ N }}**nu):($4) w xyerr t "{{ N }}", \
{% endfor %}

{% endblock content %}
