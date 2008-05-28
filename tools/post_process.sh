#!/bin/sh

FILES="gx-connection-gen.c \
        gx-connection-gen.h \
        gx-drawable-gen.c \
        gx-drawable-gen.h \
        gx-window-gen.c \
        gx-window-gen.h"

for i in $FILES
do
        sed -i 's/COLORMAP/ColorMap/g' $i
	sed -i 's/CURSOR/Cursor/g' $i
	sed -i 's/FONT/Font/g' $i
	sed -i 's/ATOM/Atom/g' $i
	sed -i 's/CONTEXT/Context/g' $i
	sed -i 's/FONTABLE/Fontable/g' $i
	sed -i 's/VISUAL/Visual/g' $i
	sed -i 's/KEY/Key/g' $i
	sed -i 's/CODE/Code/g' $i
	sed -i 's/SYM/Sym/g' $i
	sed -i 's/BUTTON/Button/g' $i
	sed -i 's/TIMESTAMP/Timestamp/g' $i
	sed -i 's/SCREEN/Screen/g' $i
	sed -i 's/DEPTH/Depth/g' $i
	sed -i 's/POINT/Point/g' $i
	sed -i 's/RECTANGLE/Rectangle/g' $i
	sed -i 's/ARC/Arc/g' $i
	sed -i 's/FORMAT/Format/g' $i
	sed -i 's/TYPE/Type/g' $i
	sed -i 's/PROP/Prop/g' $i
	sed -i 's/CHAR/Char/g' $i
	sed -i 's/INFO/Info/g' $i
	sed -i 's/SEGMENT/Segment/g' $i
        #FIXME HACKS
        sed -i 's/gx_connection_set_property/gx_connection_set_xproperty/g' $i
        sed -i 's/gx_connection_get_property/gx_connection_get_xproperty/g' $i
	indent $i
done
