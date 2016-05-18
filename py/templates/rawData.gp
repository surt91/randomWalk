{% extends "base.gp" %}

{% block content %}

{% for N in number_of_steps %}
    {{ header(filename+N|string, xlabel, ylabel) }}

    plot \
    {% for theta in thetas[N] %}
        "< zcat {{ raw }}/{{ makebase(basetheta, steps=N, theta=theta) }}.dat.gz" every 100 u 1:{{ observable + 1 }} w l t "{{ theta }}", \
    {% endfor %}
{% endfor %}

{% endblock content %}
