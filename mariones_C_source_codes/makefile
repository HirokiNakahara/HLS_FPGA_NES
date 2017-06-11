CC = gcc
CFLAGS = -O3
OPS =
OS = ULTRA
BINDIR = .#

MARIONES = main.c mariones_top.c reset_nes.c mbc.c registers.c RICOH2A03.c ppu.c

mariones : ${MARIONES}
	${CC} $(CFLAGS) ${MARIONES} -pg -lm -o ${BINDIR}/mariones

main.o : mariones.h
${MARIONES} : mariones.h

.c.o : 
	$(CC) $(CFLAGS) -D$(OS) $(OPS) -c $<
