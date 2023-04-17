DEBUG_FLAGS   = -D _DEBUG -ggdb3 -std=c++2a -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wstack-usage=8192 -pie -fPIE -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,nonnull-attribute,leak,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
FAST_FLAGS    = -march=native
SLOW_FLAGS	  = -O0
AVX_256_FLAGS = -mavxf -march=native
AVX_512_FLAGS = -mavx512f -march=tigerlake
SFML_FLAGS    = -lsfml-graphics -lsfml-window -lsfml-system

debug: debug_man_set debug_alpha_blending debug_parse_bmp debug_calloc debug_man_set
	g++ $(DEBUG_FLAGS) Src/main.cpp Obj/DrawMandel.o Obj/AlphaBlending.o Obj/ParseBmp.o Obj/AlignedCalloc.o -o Exe/Run $(SFML_FLAGS)
fast: fast_man_set fast_alpha_blending fast_parse_bmp fast_calloc
	g++ $(FAST_FLAGS) Src/main.cpp Obj/DrawMandel.o Obj/AlphaBlending.o Obj/ParseBmp.o Obj/AlignedCalloc.o -o Exe/Run $(SFML_FLAGS)
avx: fast_calloc fast_parse_bmp fast_alpha_blending_AVX fast_alpha_blending fast_man_set
	g++ $(FAST_FLAGS) $(AVX_512_FLAGS) Src/main.cpp Obj/AlphaBlendingAVX.o Obj/AlphaBlending.o Obj/ParseBmp.o Obj/AlignedCalloc.o -o Exe/Run $(SFML_FLAGS)
avx_debug: debug_calloc debug_parse_bmp debug_alpha_blending_AVX debug_man_set
	g++ $(DEBUG_FLAGS) $(AVX_512_FLAGS) Src/main.cpp Obj/AlphaBlendingAVX.o Obj/ParseBmp.o Obj/AlignedCalloc.o -o Exe/Run $(SFML_FLAGS)
	
avx256:fast_calloc fast_alpha_blending_AVX256 fast_parse_bmp 
	g++ $(FAST_FLAGS) $(AVX_FLAGS) Src/main.cpp Obj/AlphaBlending256.o Obj/ParseBmp.o Obj/AlignedCalloc.o -o Exe/Run $(SFML_FLAGS)

debug_alpha_blending: debug_calloc fast_parse_bmp fast_calloc
	g++ -c $(SLOW_FLAGS) Src/Libs/AlphaBlending/AlphaBlending.cpp Obj/AlignedCalloc.o -o Obj/AlphaBlending.o
fast_alpha_blending: fast_calloc
	g++ -c $(FAST_FLAGS) Src/Libs/AlphaBlending/AlphaBlending.cpp Obj/AlignedCalloc.o -o Obj/AlphaBlending.o

debug_alpha_blending_AVX: debug_calloc
	g++ -c $(SLOW_FLAGS) $(AVX_512_FLAGS) Src/Libs/AlphaBlendingAVX/AlphaBlendingAVX.cpp Obj/AlignedCalloc.o -o Obj/AlphaBlendingAVX.o
fast_alpha_blending_AVX: fast_calloc
	g++ -c $(FAST_FLAGS) $(AVX_512_FLAGS) Src/Libs/AlphaBlendingAVX/AlphaBlendingAVX.cpp Obj/AlignedCalloc.o -o Obj/AlphaBlendingAVX.o

fast_alpha_blending_AVX256: fast_calloc
	g++ -c $(FAST_FLAGS) $(AVX_512_FLAGS) Src/Libs/AlphaBlending256/AlphaBlending256.cpp Obj/AlignedCalloc.o -o Obj/AlphaBlending256.o

debug_parse_bmp: debug_calloc
	g++ -c $(DEBUG_FLAGS) Src/Libs/ParseBmp/ParseBmp.cpp Obj/AlignedCalloc.o -o Obj/ParseBmp.o
fast_parse_bmp: fast_calloc
	g++ -c $(FAST_FLAGS) Src/Libs/ParseBmp/ParseBmp.cpp Obj/AlignedCalloc.o -o Obj/ParseBmp.o

debug_calloc:
	g++ -c $(DEBUG_FLAGS) Src/Libs/AlignedCalloc/AlignedCalloc.cpp -o Obj/AlignedCalloc.o
fast_calloc:
	g++ -c $(FAST_FLAGS) Src/Libs/AlignedCalloc/AlignedCalloc.cpp -o Obj/AlignedCalloc.o

debug_man_set:
	g++ -c $(DEBUG_FLAGS) $(AVX_512_FLAGS) Src/Libs/DrawMandelbrot/DrawMandelbrot.cpp -o Obj/DrawMandel.o
fast_man_set:
	g++ -c $(FAST_FLAGS) $(AVX_512_FLAGS) Src/Libs/DrawMandelbrot/DrawMandelbrot.cpp -o Obj/DrawMandel.o

chat_gpt:
	g++ Src/ChatGPTVersion.cpp -o Exe/DrawMandel_Chat $(SFML_FLAGS)