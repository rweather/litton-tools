
.PHONY: all clean

LITTON_AS = ../build/src/litton-as

PROGRAMS = \
	fibonacci.drum \
	hellorld.drum \
	hello_world.drum \

all: $(PROGRAMS)

fibonacci.drum: fibonacci.las
	$(LITTON_AS) -o fibonacci.drum fibonacci.las

hello_world.drum: hello_world.las
	$(LITTON_AS) -o hello_world.drum hello_world.las

hellorld.drum: hellorld.las
	$(LITTON_AS) -o hellorld.drum hellorld.las

clean:
	rm -f $(PROGRAMS)
