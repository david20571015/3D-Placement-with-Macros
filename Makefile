all: project

.PHONY: project project-clean
project:
	$(MAKE) -C src
project-clean:
	$(MAKE) -C src clean

.PHONY: test test-clean
test:
	$(MAKE) -C test
test-clean:
	$(MAKE) -C test clean

.PHONY: clean
clean: project-clean test-clean
