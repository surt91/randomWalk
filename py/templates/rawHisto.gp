{% extends "base.gp" %}

{% block content %}

set yr [10:]

{% for N in number_of_steps %}
    {{ header(filename+"_log_"+N|string, xlabel, ylabel) }}

    set log xy
    plot \
    {% for theta in thetas[N] %}
        "{{ path }}/hist_{{ makebase(basetheta, steps=N, theta=theta) }}.dat" u 1:3 w p t "{{ theta }}", \
    {% endfor %}

    {{ header(filename+N|string, xlabel, ylabel) }}

    unset log
    set log y
    plot \
    {% for theta in thetas[N] %}
        "{{ path }}/hist_{{ makebase(basetheta, steps=N, theta=theta) }}.dat" u 1:3 w p t "{{ theta }}", \
    {% endfor %}
{% endfor %}

{% endblock content %}
