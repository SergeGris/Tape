
SOURCES = \
        src/main.cc \
        src/configuration.cc

OPT?=-O2 -fPIE -pipe -ffunction-sections -fdata-sections -fstack-protector-all
CXXFLAGS=-Wall -Wextra $(OPT)
DEPSDIR:=.deps

all: tape-sort tests

.cc.o: $(@:.o=.cc)
	@mkdir -p $(DEPSDIR)
	@echo -e "  CC\t$(@:.o=.cc)"
	@$(CXX) $(CXXFLAGS) -c -o $@ -MMD -MP -MF $(DEPSDIR)/"$(@F:.o=.d)" $(@:.o=.cc)

DEPS:=$(wildcard $(DEPSDIR)/*.d)

-include $(DEPS)

tape-sort: $(SOURCES:.cc=.o)
	@echo -e "  CCLD\t$@"
	@$(CXX) $(CXXFLAGS) -o $@ $^

tests: tape-sort
	@make -C tests
