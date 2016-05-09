all: randomWalk test

.DELETE_ON_ERROR:
.PHONY: clean proper

randomWalk: $(shell find src | sed 's/ /\\ /g')
	$(MAKE) -C src
	cp -p src/$@ $@

test: randomWalk
	touch test
	./$< -b

proper:
	$(MAKE) proper -C src

clean: proper
	rm -rf randomWalk test
	$(MAKE) clean -C src
