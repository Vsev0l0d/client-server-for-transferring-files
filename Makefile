TAGGET = main
CC = gcc

PREF_SRC = ./src/
PREF_OBJ = ./obj/

SRC = ${wildcard ${PREF_SRC}*.c}
OBJ = ${patsubst ${PREF_SRC}%.c, ${PREF_OBJ}%.o, ${SRC}}

${TAGGET}: ${OBJ}
	${CC} ${OBJ} -o ${TAGGET} -lpthread

${PREF_OBJ}%.o: ${PREF_SRC}%.c ${PREF_OBJ}
	${CC} -c $< -o $@

${PREF_OBJ}:
	mkdir ${PREF_OBJ}

clean:
	rm -fR ${PREF_OBJ} ${TAGGET}
