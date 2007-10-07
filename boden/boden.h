/*
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project and may not be used
 * in other projects without written permission of the author.
 */

#ifndef boden_boden_h
#define boden_boden_h

#include "grund.h"

/**
 * Der Boden ist der 'Natur'-Untergrund in Simu. Er kann einen Besitzer haben.
 *
 * @author Hj. Malthaner
 */

class boden_t : public grund_t
{
protected:
	static bool show_grid;

public:
	boden_t(karte_t *welt, loadsave_t *file);
	boden_t(karte_t *welt, koord3d pos, hang_t::typ slope);

	inline bool ist_natur() const { return !hat_wege(); }

	void calc_bild();

	const char *gib_name() const;

	grund_t::typ gib_typ() const {return boden;}
};

#endif
