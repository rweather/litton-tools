
.PHONY: all clean

LITTON_AS = ../build/src/litton-as

PROGRAMS = \
	echo.drum \
	fibonacci.drum \
	hellorld.drum \
	hello_world.drum \
	hello_world_pointers.drum \
	math_fragments.drum \
	fibonacci.tape \
	hellorld.tape \
	hello_world.tape \

all: $(PROGRAMS)

echo.drum: echo.las
	$(LITTON_AS) -o echo.drum echo.las

fibonacci.drum: fibonacci.las
	$(LITTON_AS) -o fibonacci.drum fibonacci.las

fibonacci.tape: fibonacci.las
	$(LITTON_AS) -T -o fibonacci.tape fibonacci.las

hello_world.drum: hello_world.las
	$(LITTON_AS) -o hello_world.drum hello_world.las

hello_world.tape: hello_world.las
	$(LITTON_AS) -T -o hello_world.tape hello_world.las

hello_world_pointers.drum: hello_world_pointers.las
	$(LITTON_AS) -o hello_world_pointers.drum hello_world_pointers.las

hellorld.drum: hellorld.las
	$(LITTON_AS) -o hellorld.drum hellorld.las

hellorld.tape: hellorld.las
	$(LITTON_AS) -T -o hellorld.tape hellorld.las

math_fragments.drum: math_fragments.las
	$(LITTON_AS) -o math_fragments.drum math_fragments.las

clean:
	rm -f $(PROGRAMS)
