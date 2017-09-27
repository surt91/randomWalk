all: randomWalk test

.DELETE_ON_ERROR:
.PHONY: clean proper cleanall test testD

randomWalk: $(shell find src | sed 's/ /\\ /g')
	$(MAKE) -C src
	cp -p src/$@ $@

randomWalkD: $(shell find src | sed 's/ /\\ /g')
	$(MAKE) debug -C src
	cp -p src/$@ $@

doc: randomWalk
	$(MAKE) doc -C src
	cp -r src/doc .

test: $(shell find src | sed 's/ /\\ /g')
testD: $(shell find src | sed 's/ /\\ /g')
test testD:
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
