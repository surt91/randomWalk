CODE	= randomWalk

all: $(CODE) test

.DELETE_ON_ERROR:
.PHONY: clean proper $(CODE) test

randomWalk:
	$(MAKE) -C src
	cp src/$@ $@

test: randomWalk | tmp
	./$< -b
	rm -rf tmp

tmp:
	mkdir $@

proper:
	$(MAKE) proper -C src

clean: proper
	rm -rf  $(CODE)
	$(MAKE) clean -C src
