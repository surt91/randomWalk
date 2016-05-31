{% extends "base.gp" %}

{% block content %}

{{ header(filename|string, xlabel, ylabel) }}

set print "phi_inf.dat"
print "# s ", "phi_inf ", "err ", "chi2"

set log y

{% for n, s in enumerate(S) %}
    f{{ n }}(x) = phi_inf{{ n }} - xi{{ n }}*x**(-gamma{{ n }})
    fit f{{ n }}(x) "{{ path }}/tran_{{ makebase(noNname+"_S{s}", s=s) }}.dat" u 1:2:3 yerr via phi_inf{{ n }}, xi{{ n }}, gamma{{ n }}
    chi{{ n }} = FIT_STDFIT
    print "{{ s }} ", phi_inf{{ n }}, phi_inf{{ n }}_err, chi{{ n }}**2
{% endfor %}

plot \
{% for n, s in enumerate(S) %}
    "{{ path }}/tran_{{ makebase(noNname+"_S{s}", s=s) }}.dat" u 1:2:3 w ye pt 1 lc {{ n+1 }} t "{{ s }}", \
    f{{ n }}(x) lc {{ n+1 }} not, \
{% endfor %}


{{ header(filename+"_fit"|string, "s", "\\Phi") }}

f(x) = a*x**b
fit f(x) "phi_inf.dat" u 1:2:3 yerr via a, b
p "phi_inf.dat", \
  f(x) t sprintf("ax^{/Symbol k}, {/Symbol k} = %.2f(%.0f), {/Symbol c} = %.1f", b, b_err*1e2, FIT_STDFIT*FIT_STDFIT)

{% endblock content %}
