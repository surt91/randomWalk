{% extends "base.gp" %}

{% block content %}

{% for N in number_of_steps %}
    {{ header(filename+N|string, xlabel, ylabel) }}

    plot \
    {% for theta in thetas[N] %}
        "{{ path }}/stiched_{{ makebase(basetheta, steps=N, theta=theta) }}.dat" u 1:3:2:4 w xyerr t "{{ theta }}", \
    {% endfor %}
{% endfor %}

{% endblock content %}
