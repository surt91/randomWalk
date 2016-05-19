{% extends "base.gp" %}

{% block content %}

{% for N in number_of_steps %}
    {{ header(filename+N|string, xlabel, ylabel) }}

    plot for [IDX=0:{{ energies[N]|length - 2 }}] "{{ path }}/wl_stiched_{{ makebase(basename, steps=N) }}.dat" index IDX u 1:2:3 with ye lt IDX+1 pt 1 t "Part ".IDX
{% endfor %}

{% endblock content %}
