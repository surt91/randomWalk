{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

Ns = "{{ number_of_steps | join(" ") }}"

plot for N in Ns "{{ path }}/stats_N".N.".dat" u 1:(1-$5/$4) w l t "acceptance ratio"

{% endblock content %}
