
MODULES_EXTERNAL_UNOISE = modules/external/unoise

INC += -I./$(MODULES_EXTERNAL_UNOISE)

SRC_MODULES_EXTERNAL += $(addprefix $(MODULES_EXTERNAL_UNOISE)/,\
	modunoise.c \
	)
