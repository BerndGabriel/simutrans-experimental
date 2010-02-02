/*
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#include "../simcolor.h"
#include "../simgraph.h"
#include "../simworld.h"
#include "../utils/cbuffer_t.h"
#include "../utils/simstring.h"
#include "ground_info.h"


static cbuffer_t gr_info(1024);


grund_info_t::grund_info_t(const grund_t* gr_) :
	gui_frame_t(gr_->get_name(), NULL),
	gr(gr_),
	view(gr_->get_welt(), gr_->get_pos(), koord( max(64, get_base_tile_raster_width()), max(56, (get_base_tile_raster_width()*7)/8) ))
{
	const ding_t* d = gr->obj_bei(0);
	if (d != NULL) {
		set_owner(d->get_besitzer());
	}
	gr_info.clear();
	gr->info(gr_info);
	sint16 height = max( (count_char(gr_info, '\n')+1)*LINESPACE+36, view.get_groesse().y+36 );

	view.set_pos( koord(165,10) );
	add_komponente( &view );

	set_fenstergroesse( koord(175+view.get_groesse().x, height) );
}



/**
 * komponente neu zeichnen. Die �bergebenen Werte beziehen sich auf
 * das Fenster, d.h. es sind die Bildschirkoordinaten des Fensters
 * in dem die Komponente dargestellt wird.
 */
void grund_info_t::zeichnen(koord pos, koord groesse)
{
	gr_info.clear();
	gr->info(gr_info);
	gui_frame_t::zeichnen(pos,groesse);
	display_multiline_text(pos.x+10, pos.y+16+10, gr_info, COL_BLACK);
}



void grund_info_t::map_rotate90( sint16 new_ysize )
{
	koord3d l = view.get_location();
	l.rotate90( new_ysize );
	view.set_location( l );
}
