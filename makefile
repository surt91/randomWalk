CODE	= randomWalk
ARTICLE = self_avoiding_ld.pdf

all: $(CODE) $(ARTICLE) test

.DELETE_ON_ERROR:
.PHONY: clean proper $(ARTICLE) $(CODE) test

self_avoiding_ld.pdf:
	$(MAKE) -C Article/self_avoiding
	cp Article/paper.pdf $@

randomWalk:
	$(MAKE) -C src
	cp src/$@ $@

test: randomWalk | tmp
	./$< --test
	rm -rf tmp

tmp:
	mkdir $@

proper:
	$(MAKE) proper -C src
	$(MAKE) proper -C Article

clean: proper
	rm -rf  $(CODE)
	$(MAKE) clean -C src
	$(MAKE) clean -C Article
