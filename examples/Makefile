
.PHONY: all clean

LITTON_AS = ../build/src/litton-as

PROGRAMS = \
	echo.drum \
	fibonacci.drum \
	hellorld.drum \
	hello_world.drum \
	hello_world_pointers.drum \

all: $(PROGRAMS)

echo.drum: echo.las
	$(LITTON_AS) -o echo.drum echo.las

fibonacci.drum: fibonacci.las
	$(LITTON_AS) -o fibonacci.drum fibonacci.las

hello_world.drum: hello_world.las
	$(LITTON_AS) -o hello_world.drum hello_world.las

hello_world_pointers.drum: hello_world_pointers.las
	$(LITTON_AS) -o hello_world_pointers.drum hello_world_pointers.las

hellorld.drum: hellorld.las
	$(LITTON_AS) -o hellorld.drum hellorld.las

clean:
	rm -f $(PROGRAMS)
