{% extends "base.gp" %}

{% block content %}

{% for N in number_of_steps %}
    {{ header(filename+N|string, xlabel, ylabel) }}

    # TODO: use a stepwise function to convert the indices to temperates
    plot \
    {% for theta in thetas[N] %}
        "< zcat swapGraph{{ N }}.dat.gz" u 1:{{ loop.index }} w l t "{{ loop.index }}", \
    {% endfor %}
{% endfor %}

{% endblock content %}
