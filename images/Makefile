
.PHONY: all

SRC_DIR = ../src/emulator-sdl

all: \
	$(SRC_DIR)/img-background.c \
	$(SRC_DIR)/img-buttons.c \
	$(SRC_DIR)/img-control-up.c \
	$(SRC_DIR)/img-control-down.c \
	$(SRC_DIR)/img-knob-A0.c \
	$(SRC_DIR)/img-knob-A8.c \
	$(SRC_DIR)/img-knob-A16.c \
	$(SRC_DIR)/img-knob-A24.c \
	$(SRC_DIR)/img-knob-A32.c \
	$(SRC_DIR)/img-knob-I0.c \
	$(SRC_DIR)/img-knob-I8.c \
	$(SRC_DIR)/img-knob-I16.c \
	$(SRC_DIR)/img-knob-I24.c \
	$(SRC_DIR)/img-knob-I32.c \
	$(SRC_DIR)/img-lamps.c \
	$(SRC_DIR)/font-dotmatrix.c

$(SRC_DIR)/img-background.c: front-panel-background.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-buttons.c: front-panel-buttons-pressed.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-lamps.c: front-panel-lamps-lit.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-control-up.c: front-panel-knob-control-up.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-control-down.c: front-panel-knob-control-down.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-A0.c: front-panel-knob-A-0.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-A8.c: front-panel-knob-A-8.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-A16.c: front-panel-knob-A-16.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-A24.c: front-panel-knob-A-24.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-A32.c: front-panel-knob-A-32.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-I0.c: front-panel-knob-I-0.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-I8.c: front-panel-knob-I-8.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-I16.c: front-panel-knob-I-16.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-I24.c: front-panel-knob-I-24.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/img-knob-I32.c: front-panel-knob-I-32.png
	echo -n 'const ' >$@
	xxd -i $< >>$@

$(SRC_DIR)/font-dotmatrix.c: ../fonts/DotMatrix-Regular.ttf
	echo -n 'const ' >$@
	xxd -i $< >>$@
