all: randomWalk test

.DELETE_ON_ERROR:
.PHONY: clean proper

randomWalk: $(shell find src | sed 's/ /\\ /g')
	$(MAKE) -C src
	cp -p src/$@ $@

randomWalkD: $(shell find src | sed 's/ /\\ /g')
	$(MAKE) debug -C src
	cp -p src/randomWalk $@

doc: randomWalk
	$(MAKE) doc -C src
	cp -r src/doc .

test: randomWalkD
	cp src/test .
	./test
	./$< -b

proper:
	$(MAKE) proper -C src

clean: proper
	rm -rf randomWalk test
	$(MAKE) clean -C src
