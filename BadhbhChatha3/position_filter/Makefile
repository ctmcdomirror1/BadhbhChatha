check:
	gcc check.c check.h check_test.c \
		-I ./ ./filter_common.c \
		-I ../
		-I ../util ../util/util.c \
		-mabm -mbmi -lm -fvar-tracking -o check_test -g

cost:
	gcc pawn_cost.c pawn_cost_test.c filter_common.c \
		-I ../dependencies/hungarian ../dependencies/hungarian/hungarian.c \
		-I ../util ../util/util.c \
		-I ../ ../chess.h \
		-mabm -mbmi -fvar-tracking -Wall -o cost_test -g

common:
	gcc filter_common_test.c filter_common.c \
		-o filter_test -g
