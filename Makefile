CFLAGS= -Wshadow    						\
	-Winit-self 						\
	-Wredundant-decls 					\
	-Wcast-align						\
	-Wundef 						\
	-Wfloat-equal 						\
	-Winline 						\
	-Wunreachable-code 					\
	-Wmissing-declarations 					\
	-Wmissing-include-dirs 					\
	-Wswitch-enum 						\
	-Wswitch-default 					\
	-Weffc++ 						\
	-Wmain 							\
	-Wextra 						\
	-Wall 							\
	-g 							\
	-pipe 							\
	-fexceptions 						\
	-Wcast-qual 						\
	-Wconversion 						\
	-Wctor-dtor-privacy 					\
	-Wempty-body 						\
	-Wformat-security 					\
	-Wformat=2 						\
	-Wignored-qualifiers 					\
	-Wlogical-op 						\
	-Wmissing-field-initializers 				\
	-Wnon-virtual-dtor 					\
	-Woverloaded-virtual 					\
	-Wpointer-arith 					\
	-Wsign-promo 						\
	-Wstack-usage=8192 					\
	-Wstrict-aliasing 					\
	-Wstrict-null-sentinel 					\
	-Wtype-limits 						\
	-Wwrite-strings 					\
	-D_DEBUG 						\
	-D_EJUDGE_CLIENT_SIDE

OPTIM_FLAGS = -O2

SANITIZE_FLAGS = -lasan -fsanitize=address,leak

TEXT_CFILES  = src/utils/text.cpp
LOG_CFILES   = src/log/log.cpp
UTILS_CFILES = src/utils/utils.cpp
JIT_CFILES   = src/jit/jit.cpp src/jit/tokens2x86.cpp src/jit/myIO.cpp src/jit/translate2x86.cpp
CODE_GEN_CFILES = src/jit/conditional_cmds.cpp src/jit/jmp_cmds.cpp src/jit/mem_cmds.cpp src/jit/math_cmds.cpp \
		  src/jit/encode_cmd.cpp
FRONTEND_CFILES = src/frontend/tokens.cpp

IR_CFILES = src/frontend/IR.cpp

CFILES = main.cpp $(TEXT_CFILES) $(LOG_CFILES) $(UTILS_CFILES) $(JIT_CFILES) \
		  $(CODE_GEN_CFILES) $(FRONTEND_CFILES) $(IR_CFILES)

OUTPUT = jit

TEST_CFILES = tests/test_encoding.cpp
TEST_OUTPUT = tests/test.out

all:
	@ clear
	@ g++ $(OPTIM_FLAGS) -masm=intel -o $(OUTPUT) $(CFLAGS) $(CFILES) -no-pie
	@ echo Compiled c-files

.PHONY: sanitize
sanitize:
	@ clear
	@ g++ $(OPTIM_FLAGS) -masm=intel -o $(OUTPUT) $(CFLAGS) $(CFILES) -no-pie $(SANITIZE_FLAGS)
	@ echo Compiled c-files

.PHONY: run
run:
	@ ./$(OUTPUT) input.txt
	@ echo Run

.PHONY: comp_debug
run_debug:
	@ @ g++ $(OPTIM_FLAGS) -masm=intel -o $(OUTPUT) $(CFLAGS) $(CFILES) -no-pie -DRUN_WITH_INT3
	@ echo Comiled with RUN_WITH_INT3 define.

.PHONY: test
test:
	@ g++ $(OPTIM_FLAGS) -o $(TEST_OUTPUT) $(CFLAGS) $(TEXT_CFILES) $(LOG_CFILES) $(UTILS_CFILES) $(JIT_CFILES) \
		  $(CODE_GEN_CFILES) $(FRONTEND_CFILES) $(IR_CFILES) $(TEST_CFILES)
	@ nasm -f elf64 tests/standart.s
	@ ld -s -o tests/standart tests/standart.o
	@ echo Complied test files.
	@ ./$(TEST_OUTPUT)
	@ echo Run tests.
