DEBUG_FLAGS = -D _DEBUG -ggdb3 -std=c++2a -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wstack-usage=8192 -pie -fPIE -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,leak,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
RELEASE_FLAGS = -O3 -DRELEASE
AVX_256_FLAGS = -O3 -DAVX256 -msse4.2 
AVX_512_FLAGS = -O3 -DAVX512 -march=sandybridge -march=haswell -march=knl -flax-vector-conversions
SFML_FLAGS  = -lsfml-graphics -lsfml-window -lsfml-system

BIN = Exe/DrawMandelbrotSet

define link
	g++ Obj/main.o Obj/DrawMandel.o -o $(BIN) $(SFML_FLAGS)
endef

avx512: prepare Obj/main.o
	g++ -c $(AVX_512_FLAGS) Src/DrawMandelbrot/DrawMandelbrot.cpp -o Obj/DrawMandel.o
	$(call link)

sse: prepare Obj/main.o 
	g++ -c $(AVX_256_FLAGS) Src/DrawMandelbrot/DrawMandelbrot.cpp -o Obj/DrawMandel.o
	$(call link)

without_simd: prepare Obj/main.o 
	g++ -c $(RELEASE_FLAGS) Src/DrawMandelbrot/DrawMandelbrot.cpp -o Obj/DrawMandel.o

Obj/main.o: Src/main.cpp
	g++ -c Src/main.cpp -o Obj/main.o

debug: prepare
	g++ -c $(DEBUG_FLAGS) Src/DrawMandelbrot/DrawMandelbrot.cpp -o Obj/DrawMandel.o
	$(call link)

run:
	$(BIN)

prepare:
	-mkdir Obj
	-mkdir Exe 