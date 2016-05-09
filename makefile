all: randomWalk test

.DELETE_ON_ERROR:
.PHONY: clean proper

randomWalk: src
	$(MAKE) -C src
	cp -p src/$@ $@

test: randomWalk
	./$< -b > test || rm test

proper:
	$(MAKE) proper -C src

clean: proper
	rm -rf randomWalk test
	$(MAKE) clean -C src
