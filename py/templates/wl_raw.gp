{% extends "base.gp" %}

{% block content %}

{% for N in number_of_steps %}
    {{ header(filename+N|string, xlabel, ylabel) }}

    plot for [IDX=0:{{ energies[N]|length - 1 }}] "{{ path }}/wl_raw_{{ makebase(basename, steps=N) }}.dat" index IDX u 1:2 with p lt IDX+1 t "Part ".IDX
{% endfor %}

{% endblock content %}
