{% extends "base.gp" %}

{% block content %}

{{ header(filename+N|string, xlabel, ylabel) }}

d = {{ dimension }}
nu = {{ "d" if observable == 2 else "(d-1)" }}*{{ nu[typ][dimension] }}

#~ f1(x) = a1*exp(-(x-mu1)**2/(2*sigma1**2))
#~ f2(x) = a2*exp(-(x-mu2)**2/(2*sigma2**2))

#~ f1(x) = a1 - (x-mu1)**2 / (2*sigma1**2)
#~ f2(x) = a2 - (x-mu2)**2 / (2*sigma2**2)

#~ {% set N=1024 %}
#~ fit [:1] f1(x) "{{ path }}/whole_{{ makebase(basename, steps=N) }}.dat" u ($1/{{ N }}**nu):($3+log({{ N }}**nu)):($2/{{ N }}**nu):($4) xyerr via a1, mu1, sigma1
#~ chi1 = FIT_STDFIT
#~ fit [1:15] f2(x) "{{ path }}/whole_{{ makebase(basename, steps=N) }}.dat" u ($1/{{ N }}**nu):($3+log({{ N }}**nu)):($2/{{ N }}**nu):($4) xyerr via a2, mu2, sigma2
#~ chi2 = FIT_STDFIT

plot \
{% for N in number_of_steps %}
    "{{ path }}/whole_{{ makebase(basename, steps=N) }}.dat" u ($1/{{ N }}**nu):($3+log({{ N }}**nu)):($2/{{ N }}**nu):($4) w xyerr t "{{ N }}", \
{% endfor %}
    #~ f2(x) t sprintf("$\\mu = %.2f(%.0f), \\sigma = %.2f(%.0f), \\chi^2 = %.1f", mu2, mu2_err*1e2, sigma2, sigma2_err*1e2, chi2**2)
    #~ f1(x) t sprintf("$\\mu = %.2f(.0f), \\sigma = %.2f(.0f), \\chi^2 = %.1f", mu1, mu1_err*1e2, sigma1, sigma1_err*1e2, chi1**2), \

{% endblock content %}
