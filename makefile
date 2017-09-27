all: randomWalk test

.DELETE_ON_ERROR:
.PHONY: clean proper cleanall test testD

randomWalk randomWalkD: $(shell find src | sed 's/ /\\ /g')
	$(MAKE) $@ -C src
	cp -p src/$@ .

doc: randomWalk
	$(MAKE) doc -C src
	cp -r src/doc .

test testD: $(shell find src | sed 's/ /\\ /g')
	$(MAKE) $@ -C src
	cp src/$@ .
	./$@

proper:
	rm bench*
	$(MAKE) proper -C src

clean: proper
	rm -rf randomWalk test randomWalkD testD
	$(MAKE) clean -C src

cleanall: clean
	$(MAKE) cleanall -C src
