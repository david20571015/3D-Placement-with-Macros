EXEC := ../main
EVALUATOR := evaluator
INPUTS := $(wildcard cases/*.txt)
OUTPUTS := $(INPUTS:%.txt=%.out)

all: test

.PHONY: test
test: $(CASES:cases/%.txt=test_%)

.PHONY: test_%
test_%: cases/%.txt cases/%.out
	@echo "Testing $<"
	@$(EVALUATOR) $^
	@echo "OK"

%.out: %.txt
	./$(EXEC) $< $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTS)
