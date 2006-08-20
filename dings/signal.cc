/*
 * signal.cc
 *
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project and may not be used
 * in other projects without written permission of the author.
 */

#include <stdio.h>

#include "../simdebug.h"
#include "../simworld.h"
#include "../simdings.h"
#include "../boden/wege/schiene.h"
#include "../boden/grund.h"
#include "../simimg.h"
#include "../dataobj/loadsave.h"
#include "../dataobj/translator.h"
#include "../utils/cbuffer_t.h"

#include "signal.h"


/**
 * @return Einen Beschreibungsstring f�r das Objekt, der z.B. in einem
 * Beobachtungsfenster angezeigt wird.
 * @author Hj. Malthaner
 */
void signal_t::info(cbuffer_t & buf) const
{
	// well, needs to be done
	ding_t::info(buf);

	buf.append(translator::translate("\ndirection:"));
	buf.append(get_dir());
}


void signal_t::calc_bild()
{
	after_bild = IMG_LEER;
	image_id bild = IMG_LEER;

	grund_t *gr = welt->lookup(gib_pos());
	if(gr) {
		weg_t * sch = gr->gib_weg(besch->gib_wtyp());
		uint16 offset=0;
		if(sch  &&  sch->is_electrified()  &&  (besch->gib_bild_anzahl()/8)>1) {
			offset = besch->is_pre_signal() ? 12 : 8;
		}

		if(dir&ribi_t::west) {
			after_bild = besch->gib_bild_nr(3+zustand*4+offset);
		}

		if(dir&ribi_t::sued) {
			if(after_bild!=IMG_LEER) {
				bild = besch->gib_bild_nr(0+zustand*4+offset);
			}
			else {
				after_bild = besch->gib_bild_nr(0+zustand*4+offset);
			}
		}

		if(dir&ribi_t::ost) {
			bild = besch->gib_bild_nr(2+zustand*4+offset);
		}

		if(dir&ribi_t::nord) {
			if(bild!=IMG_LEER) {
				after_bild = besch->gib_bild_nr(1+zustand*4+offset);
			}
			else {
				bild = besch->gib_bild_nr(1+zustand*4+offset);
			}
		}

	}
	setze_bild(0,bild);
}



// establish direction
void signal_t::laden_abschliessen()
{
	step_frequency = 0;
	set_dir(dir);
	calc_bild();
}
