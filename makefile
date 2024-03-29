all: randomWalk test

.DELETE_ON_ERROR:
.PHONY: clean proper cleanall test testD bench

randomWalk randomWalkD: $(shell find src | sed 's/ /\\ /g')
	$(MAKE) $@ -C src
	cp -p src/$@ .

doc: randomWalk
	$(MAKE) doc -C src
	cp -r src/doc .

test testD bench: $(shell find src | sed 's/ /\\ /g')
	$(MAKE) $@ -C src
	cp src/$@ .
	./$@

proper:
	rm -f bench*
	$(MAKE) proper -C src

clean: proper
	rm -rf randomWalk test randomWalkD testD
	$(MAKE) clean -C src

cleanall: clean
	$(MAKE) cleanall -C src
