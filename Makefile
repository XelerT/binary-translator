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
JIT_CFILES   = src/jit/jit.cpp src/jit/tokens2x86.cpp
FRONTEND_CFILES = src/frontend/tokens.cpp

CFILES = main.cpp $(TEXT_CFILES) $(LOG_CFILES) $(UTILS_CFILES) $(JIT_CFILES) $(FRONTEND_CFILES)

OUTPUT = jit.out

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
