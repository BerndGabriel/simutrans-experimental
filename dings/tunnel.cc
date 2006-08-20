/*
 * tunnel.cc
 *
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project and may not be used
 * in other projects without written permission of the author.
 */

#include <stdio.h>

#include "../simworld.h"
#include "../simdings.h"
#include "../simplay.h"
#include "../simwerkz.h"
#include "../boden/grund.h"
#include "../boden/wege/strasse.h"
#include "../boden/wege/schiene.h"
#include "../simimg.h"
#include "../utils/cbuffer_t.h"
#include "../bauer/tunnelbauer.h"

#include "../dataobj/loadsave.h"

#include "../besch/tunnel_besch.h"

#include "tunnel.h"




tunnel_t::tunnel_t(karte_t *welt, loadsave_t *file) : ding_t(welt)
{
  besch = 0;
  rdwr(file);
  step_frequency = 0;
}


tunnel_t::tunnel_t(karte_t *welt, koord3d pos,
		   spieler_t *sp, const tunnel_besch_t *besch) :
    ding_t(welt, pos)
{
	this->besch = besch;
	setze_besitzer( sp );
	step_frequency = 0;
	calc_bild();
}


/**
 * @return Einen Beschreibungsstring f�r das Objekt, der z.B. in einem
 * Beobachtungsfenster angezeigt wird.
 * @author Hj. Malthaner
 */
void tunnel_t::info(cbuffer_t & buf) const
{
  ding_t::info(buf);
}



void
tunnel_t::calc_bild()
{
	if(besch) {
		const grund_t *gr = welt->lookup(gib_pos());
		setze_bild( 0, besch->gib_hintergrund_nr(gr->gib_grund_hang()) );
		after_bild = besch->gib_vordergrund_nr(gr->gib_grund_hang());
	}
}




void tunnel_t::rdwr(loadsave_t *file)
{
	ding_t::rdwr(file);
}


void tunnel_t::laden_abschliessen()
{
	const grund_t *gr = welt->lookup(gib_pos());
	if(gr->gib_grund_hang()==0) {
		step_frequency = 1;	// remove
		dbg->error("tunnel_t::laden_abschliessen()","tunnel entry at flat position at %i,%i will be removed.",gib_pos().x,gib_pos().y);
	}

	if(gr->gib_weg(weg_t::strasse)) {
		besch = tunnelbauer_t::strassentunnel;
	}
	else {
		besch = tunnelbauer_t::schienentunnel;
	}
	calc_bild();
}
