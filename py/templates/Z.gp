{% extends "base.gp" %}

{% block content %}

{% for N in number_of_steps %}
    set output "trash.tmp"

    # Retrieve statistical properties
    # See: http://www.phyast.pitt.edu/~zov1/gnuplot/html/statistics.html
    {% for theta in thetas[N] %}
        {% set ts = theta | string | replace("-", "_") %}
        fname_{{ ts }} = "{{ path }}/Z_m{{ sampling }}_t{{ typ }}_w{{ observable }}_d{{ dimension }}_N{{ N }}_n{{ iterations }}_x{{ seedMC }}_y{{ seedR }}_T{{ "%.5f" % theta }}.dat"
        # test if the file is empty
        stat = system("wc ".fname_{{ ts }})
        if (stat ne "" &&  int(word(stat,1)) > 3) {
            plot fname_{{ ts }} u 1:2
            f(x) = mean_y_{{ ts }}
            fit f(x) fname_{{ ts }} u 1:2 via mean_y_{{ ts }}
        }
    {% endfor %}

    unset xr
    unset yr

    {{ header(filename+N|string, xlabel, ylabel) }}

    plot \
    {% for theta in thetas[N] %}
        {% set ts = theta | string | replace("-", "_") %}
        fname_{{ ts }} u 1:($2-mean_y_{{ ts }}):3 w yerr t "{{ theta }}", \
    {% endfor %}

{% endfor %}

{% endblock content %}
