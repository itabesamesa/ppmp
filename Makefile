CC = gcc

objects = \
	utils/misc.o \
	utils/pngtopng.o \
	utils/edge_detect.o \
	utils/my_math.o \
	utils/blur.o \
	utils/scale.o \
	utils/flip.o \
	utils/skew.o \
	utils/rotate.o \
	utils/mask.o \
	utils/posterize.o \
	utils/noise.o \
	utils/colors.o \
	utils/select.o \
	utils/blend.o \
	utils/stl/load_stl.o

misc = \
	utils/misc.h

headers = \
	$(misc) \
	utils/pngtopng.h \
	utils/edge_detect.h \
	utils/my_math.h \
	utils/blur.h \
	utils/scale.h \
	utils/flip.h \
	utils/skew.h \
	utils/rotate.h \
	utils/mask.h \
	utils/posterize.h \
	utils/noise.h \
	utils/colors.h \
	utils/select.h \
	utils/blend.h \
	utils/stl/load_stl.h

displayh = \
	utils/display.h

displayo = \
	utils/display.o

wmh = \
	utils/wm.h \
	utils/text.h \
	utils/parser.h
	#utils/displaywm.h \

wmo = \
	utils/wm.o \
	utils/text.o \
	utils/parser.o
	#utils/displaywm.o \

.PHONY: wl
wl: g.out

g.out: gui3.o $(objects) $(wmo) utils/wlgui.o $(headers) $(wmh)
	$(CC) gui3.o $(objects) $(wmo) utils/wlgui.o utils/xdg-shell-protocol.o -lpng -lm -lwayland-client -lrt -lxkbcommon -lcairo -o g.out

.PHONY: X11
X11: f.out

f.out: gui3.o $(objects) $(wmo) utils/displaywm.o utils/X11gui.o $(headers) $(wmh)
	$(CC) gui3.o $(objects) $(wmo) utils/displaywm.o utils/X11gui.o -lpng -lX11 -lm -o f.out

.PHONY: gui2
e.out: gui2.o $(objects) $(wmo)
	$(CC) gui2.o $(objects) $(wmo) utils/displaywm.o -lpng -lX11 -lm -o e.out

.PHONY: gui
a.out: gui.o $(objects) $(displayo)
	$(CC) gui.o $(objects) $(displayo) -lpng -lX11 -lm -o a.out

utils/misc.o: utils/misc.c utils/pngtopng.h
	$(CC) -c utils/misc.c -lpng -lm -o utils/misc.o
utils/pngtopng.o: utils/pngtopng.c $(misc)
	$(CC) -c utils/pngtopng.c -lpng -o utils/pngtopng.o
utils/display.o: utils/display.c $(misc) utils/stl/load_stl.h utils/my_math.h
	$(CC) -c utils/display.c -lpng -lX11 -o utils/display.o
utils/edge_detect.o: utils/edge_detect.c $(misc)
	$(CC) -c utils/edge_detect.c -lm -o utils/edge_detect.o
utils/my_math.o: utils/my_math.c $(misc)
	$(CC) -c utils/my_math.c -lm -o utils/my_math.o
utils/blur.o: utils/blur.c $(misc) utils/edge_detect.h
	$(CC) -c utils/blur.c -o utils/blur.o
utils/scale.o: utils/scale.c $(misc) utils/blur.h
	$(CC) -c utils/scale.c -lm -o utils/scale.o
utils/flip.0: utils/flip.c $(misc)
	$(CC) -c utils/flip.c -o utils/flip.o
utils/skew.o: utils/skew.c $(misc)
	$(CC) -c utils/skew.c -lm -o utils/skew.o
utils/rotate.o: utils/rotate.c $(misc) utils/skew.h
	$(CC) -c utils/rotate.c -lm -o utils/rotate.o
utils/mask.o: utils/mask.c $(misc)
	$(CC) -c utils/mask.c -o utils/mask.o
utils/posterize.o: utils/posterize.c $(misc)
	$(CC) -c utils/posterize.c -o utils/posterize.o
utils/noise.o: utils/noise.c $(misc)
	$(CC) -c utils/noise.c -o utils/noise.o
utils/colors.o: utils/colors.c $(misc)
	$(CC) -c utils/colors.c -o utils/colors.o
utils/select.o: utils/select.c $(misc)
	$(CC) -c utils/select.c -lm -o utils/select.o
utils/blend.o: utils/blend.c $(misc)
	$(CC) -c utils/blend.c -lm -o utils/blend.o
utils/stl/load_stl.o: utils/stl/load_stl.c
	$(CC) -c utils/stl/load_stl.c -lm -o utils/stl/load_stl.o
gui.o: gui.c $(headers) $(displayh)
	$(CC) -c gui.c -lpng -lX11 -lm -o gui.o

utils/wm.o: utils/wm.c $(misc) $(wmh)
	$(CC) -c utils/wm.c -o utils/wm.o
gui2.o: gui2.c $(headers) $(wmh)
	$(CC) -c gui2.c -lpng -lX11 -lm -o gui2.o
utils/displaywm.o: utils/displaywm.c $(misc) utils/wm.h utils/my_math.h utils/colors.h
	$(CC) -c utils/displaywm.c -lpng -lX11 -o utils/displaywm.o
utils/text.o: utils/text.c $(misc)
	$(CC) -c utils/text.c -o utils/text.o
utils/parser.o: utils/parser.c $(misc)# utils/functions.h
	$(CC) -c utils/parser.c -o utils/parser.o

utils/X11gui.o: utils/X11gui.c $(misc) $(wmh) utils/displaywm.h utils/gui.h
	$(CC) -c utils/X11gui.c -lX11 -lm -o utils/X11gui.o
utils/wlgui.o: utils/wlgui.c $(misc) $(wmh) utils/gui.h
	$(CC) -c utils/wlgui.c -lwayland-client -lrt -lxkbcommon -lcairo -o utils/wlgui.o
#$(CC) utils/wlguistuff.o utils/xdg-shell-protocol.o -lwayland-client -lrt -lxkbcommon -o utils/wlgui.o
gui3.o: gui3.c $(headers) $(wmh)
	$(CC) -c gui3.c -lpng -lm -lxkbcommon -o gui3.o

.PHONY: clean
clean: a.out $(objects)
	-rm a.out $(objects)

.PHONY: gpt
gpt:
	$(CC) generate_parser_txt.c -lm -o b.out

.PHONY: parser
parser: parser.c $(headers) utils/gui.h $(objects) utils/gui.o
	$(CC) -c utils/gui.c -lpng -lX11 -lm -o utils/gui.o
	$(CC) -c parser.c -lpng -lX11 -lm -o parser.o
	$(CC) parser.o $(objects) utils/gui.o -lpng -lX11 -lm -o c.out

.PHONY: wm
wm: wm.c $(misc)
	$(CC) -c wm.c -lX11 -lm -o wm.o
	$(CC) wm.o utils/misc.o -lX11 -lm -o d.out
