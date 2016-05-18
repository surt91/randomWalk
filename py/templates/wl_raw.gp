{% extends "base.gp" %}

{% block content %}

{% for N in number_of_steps %}
    {{ header(filename+N|string, xlabel, ylabel) }}

    plot for [IDX=0:{{ number_of_steps|length - 1 }}] "{{ path }}/wl_raw_m{{ sampling }}_t{{ typ }}_w{{ observable }}_d{{ dimension }}_N{{ N }}_n{{ iterations }}_x{{ seedMC }}_y{{ seedR }}_T0.00000.dat" index IDX u 1:2 with p lt IDX+1 t "Part ".IDX
{% endfor %}

{% endblock content %}
