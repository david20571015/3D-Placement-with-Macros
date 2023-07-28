EXEC_PATH := $(addprefix ../,$(EXEC))
EVALUATOR := ./evaluator
INPUTS := $(wildcard cases/*.txt)
OUTPUTS := $(INPUTS:%.txt=%.out)

all: test

.PHONY: test
test: $(INPUTS:cases/%.txt=test_%)

.PHONY: test_%
test_%: cases/%.txt cases/%.out
	@echo "Testing $<"
	@$(EVALUATOR) $^

%.out: %.txt $(EXEC_PATH)
	./$(EXEC_PATH) $< $@ >/dev/null 2>&1

.PHONY: clean
clean:
	$(RM) $(OUTPUTS)
