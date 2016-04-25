{% macro header(filename, xlabel="", ylabel="", cblabel=None) %}

set output "{{ filename }}.eps"

set key samplen 1.

set xl "{{ xlabel }}"
set yl "{{ ylabel }}"

{% if cblabel %}
set cblabel "{{cblabel}}"
{% endif %}

{% endmacro %}
