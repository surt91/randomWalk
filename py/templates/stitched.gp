{% extends "base.gp" %}

{% block content %}

{% for N in number_of_steps %}
    {{ header(filename+N|string, xlabel, ylabel) }}

    plot \
    {% for theta in thetas[N] %}
        "{{ path }}/stitch_{{ makebase(basetheta, steps=N, theta=theta) }}.dat" u 1:3 w p t "{{ theta }}", \
    {% endfor %}
{% endfor %}

{% endblock content %}
