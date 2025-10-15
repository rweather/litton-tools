;
; 1-Dimensional Game of Life.
;
; Based on: https://jonmillen.com/1dlife/index.html
;
; Rules:
;
; 1. Each cell X is the center of a pattern YYXYY where the Y cells are
;    X's neighbours.
; 2. If X is dead, then it comes alive if it has 2 or 3 live neighbours.
; 3. If X is alive, then it dies if it has 2 or 4 live neighbours.
;
; In this version the playing field is 40 cells wide and 32 generations
; will be calculated.
;
; Scratchpad loop variables:
;
;   S0-S5 - Temporaries
;   S6 - Bit pattern for the current generation
;   S7 - Generation counter
;
    title "1D Game of Life"
    printer $41,"EBS1231"
    drumsize 4096
    org $800
start:
    isw printer
    oiw "\r"
    oiw "\n"
;
    ca const_generation_counter
    st 7
    ca const_start_pattern
    st 6
;
; Draw the current generation as a line of live and dead cells.
;
draw:
    ca 6
    st 1
    ca const_cells_counter
draw_loop:
    bld 1
    jc draw_live
    oiw $BB                 ; '.' for a dead cell.
    ju draw_loop_next
draw_live:
    oiw $0B                 ; '#' for a live cell.
draw_loop_next:
    sk
    ak
    tz
    jc draw_done
    ju draw_loop
draw_done:
    oiw "\r"                ; Print CRLF at the end of the line.
    oiw "\n"

;
; Have we finished all of the generations yet?
;
    ca 7
    sk
    ak
    st 7
    jc $7FF             ; Return to OPUS after the last generation.

;
; Compute the cells that live or die in the next generation.
;
    ca const_cells_counter
    st 5
    ca 6                ; Construct a 80-bit word with two zero bits
    st 1                ; at the top, followed by the 40 bits of the
    cl                  ; previous generation, and then 38 zero bits.
    st 0
    brd 2
count_neighbours:
    ca 0                ; Copy S0/S1 to S2/S3 to save it.
    st 2
    ca 1
    st 3
    cl                  ; Count the neighbours into A.
    bld 1
    ak
    bld 1
    ak
    bld 1               ; Is the cell itself alive or dead?
    jc cell_alive
    bld 1               ; Count the rest of the neighbours of a dead cell.
    ak
    bld 1
    ak
;
; Cell is dead.  It will come alive again if the neighbour count is 2 or 3,
; or stay dead otherwise.
;
    ad const_neg_2
    tz
    jc cell_new
    ad const_neg_1
    tz
cell_new:
    ca 6                ; K is 1 if the cell is now alive, 0 if dead.
    blsk 1
    st 6                ; Shift the cell's new state into S6.
;
    ca 2                ; Shift S2/S3 up by 1 bit and copy to S0/S1.
    bls 1
    st 0
    ca 3
    blsk 1
    st 1
;
    ca 5                ; If the counter in S5 has incremented to 0,
    sk                  ; then we are done with this generation.
    ak
    st 5
    jc draw
    ju count_neighbours ; Otherwise count the neighbours of the next cell.
;
; Cell is alive.  It will stay alive if the neighbour count is 2 or 4,
; or die otherwise.
;
cell_alive:
    bld 1               ; Count the rest of the neighbours of a live cell.
    ak
    bld 1
    ak
    ad const_neg_2      ; Is the count 2 or 4?
    tz
    jc cell_new
    ad const_neg_2
    tz
    ju cell_new

const_neg_2:
    dw -2
const_neg_1:
    dw -1
const_cells_counter:
    dw -40
const_generation_counter:
    dw -32
const_start_pattern:
    dw $0000770000      ; Starting pattern to generate a "face".

    entry start
