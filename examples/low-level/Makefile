
.PHONY: all clean

LITTON_AS = ../../build/src/litton-as

PROGRAMS = \
	echo.drum \
	fibonacci.drum \
	hellorld.drum \
	hello_world.drum \
	hello_world_pointers.drum \
	math_fragments.drum \
	mandelbrot.drum \
	life1d.drum \
	fibonacci.tape \
	hellorld.tape \
	hello_world.tape \
	mandelbrot.tape \
	life1d.tape \

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

mandelbrot.drum: mandelbrot.las
	$(LITTON_AS) -o mandelbrot.drum mandelbrot.las

mandelbrot.tape: mandelbrot.las
	$(LITTON_AS) -T -o mandelbrot.tape mandelbrot.las

life1d.drum: life1d.las
	$(LITTON_AS) -o life1d.drum life1d.las

life1d.tape: life1d.las
	$(LITTON_AS) -T -o life1d.tape life1d.las

clean:
	rm -f $(PROGRAMS)
