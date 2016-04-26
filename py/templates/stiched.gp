{% extends "base.gp" %}

{% block content %}

{% for N in number_of_steps %}
    {{ header(filename+N|string, xlabel, ylabel) }}

    plot \
    {% for theta in thetas[N] %}
        "{{ path }}/stiched_m{{ sampling }}_t{{ typ }}_w{{ observable }}_d{{ dimension }}_N{{ N }}_n{{ iterations }}_x{{ seedMC }}_y{{ seedR }}_T{{ "%.5f" % theta }}.dat" u 1:3:2:4 w xyerr t "{{ theta }}", \
    {% endfor %}
{% endfor %}

{% endblock content %}
