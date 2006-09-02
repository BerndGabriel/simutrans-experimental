/*
 * blockmanager.cc
 *
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project and may not be used
 * in other projects without written permission of the author.
 */

#include <stdio.h>

#include "simdebug.h"
#include "simworld.h"
#include "simplay.h"
#include "simmesg.h"
#include "simvehikel.h"
#include "dings/signal.h"
#include "dings/tunnel.h"
#include "boden/grund.h"
#include "boden/wege/schiene.h"

#include "dataobj/loadsave.h"
#include "dataobj/umgebung.h"

#include "tpl/slist_tpl.h"
#include "tpl/array_tpl.h"

#include "old_blockmanager.h"

// only needed for loading old games
class oldsignal_t : public ding_t
{
protected:
	uint8 zustand;
	uint8 blockend;
	uint8 dir;
	ding_t::typ type;

public:
	oldsignal_t(karte_t *welt, loadsave_t *file, ding_t::typ type);

	/*
	* return direction or the state of the traffic light
	* @author Hj. Malthaner
	*/
	ribi_t::ribi get_dir() const 	{ return dir; }

	bool ist_blockiert() const {return blockend != 0;}

	ding_t::typ gib_typ() const 	{ return type; }

	void rdwr(loadsave_t *file);
};


static slist_tpl <oldsignal_t *> signale;

//------------------------- old blockmanager ----------------------------
// only there to convert old games to 89.02 and higher

// these two routines for compatibility
oldsignal_t::oldsignal_t(karte_t *welt, loadsave_t *file, ding_t::typ type) : ding_t (welt)
{
	this->type = type;
	rdwr(file);
}

void
oldsignal_t::rdwr(loadsave_t *file)
{
	if(!file->is_loading()) {
		dbg->fatal("oldsignal_t::rdwr()","cannot be saved!");
	}
	// loading from blockmanager!
	ding_t::rdwr(file);
	file->rdwr_byte(blockend, " ");
	file->rdwr_byte(zustand, " ");
	file->rdwr_byte(dir, "\n");
}



// now the old block reader
void
old_blockmanager_t::rdwr_block(karte_t *welt,loadsave_t *file)
{
	int count;
	short int typ = ding_t::signal;

	// signale laden
	file->rdwr_delim("S ");
	file->rdwr_long(count, "\n");

	for(int i=0; i<count; i++) {
		// read the old signals (only opurpose of the here
		typ=file->rd_obj_id();
		oldsignal_t *sig = new oldsignal_t(welt, file, (ding_t::typ)typ);
		DBG_MESSAGE("oldsignal_t()","on %i,%i with dir=%i blockend=%i",sig->gib_pos().x,sig->gib_pos().y,sig->get_dir(),sig->ist_blockiert());
		signale.insert( sig );
	}

	// counters
	if(file->get_version()<=88005) {
		// old style
		long dummy = 0;
		file->rdwr_long(dummy, " ");
		file->rdwr_long(dummy, "\n");
	}
	else  {
		short dummy;
		file->rdwr_short(dummy, "\n");
	}
}



void
old_blockmanager_t::rdwr(karte_t *welt, loadsave_t *file)
{
	signale.clear();
	if(file->get_version()>=89000) {
		// nothing to do any more ...
		return;
	}

	if(file->is_saving()) {
		dbg->fatal("old_blockmanager::rdwr","loading only");
	}
	// this routine just reads the of signal positions
	// and converts them to the new type>
	int count;
	file->rdwr_long(count, "\n");
	for(int i=0; i<count; i++) {
		rdwr_block(welt,file);
	}
	DBG_MESSAGE("old_blockmanager::rdwr","%d old signals loaded",count);
}



void
old_blockmanager_t::laden_abschliessen(karte_t *welt)
{
	DBG_MESSAGE("old_blockmanager::laden_abschliessen()","convert olf to new signals" );
	char buf[256];
	const char *err_text=translator::translate("Error restoring old signal near (%i,%i)!");
	int failure=0;
	while(signale.count()>0) {
		oldsignal_t *os1=signale.remove_first();
		oldsignal_t *os2=NULL;
		grund_t *gr=welt->lookup(os1->gib_pos());
		grund_t *to=NULL;
		uint8 directions=0;
		weg_t::typ wt=gr->gib_weg(weg_t::schiene) ? weg_t::schiene : weg_t::monorail;
		if(gr->get_neighbour(to,wt,koord((ribi_t::ribi)os1->get_dir()))) {
			slist_iterator_tpl<oldsignal_t *> iter(signale);
			while(iter.next()) {
				if(iter.get_current()->gib_pos()==to->gib_pos()) {
					os2 = iter.get_current();
					break;
				}
			}
			if(os2==NULL) {
				dbg->error("old_blockmanager_t::laden_abschliessen()","old signal near (%i,%i) is unpaired!",gr->gib_pos().x,gr->gib_pos().y);
				message_t::get_instance()->add_message(translator::translate("Orphan signal during loading!"),os1->gib_pos().gib_2d(),message_t::problems);
			}
		}
		else {
			dbg->error("old_blockmanager_t::laden_abschliessen()","old signal near (%i,%i) is unpaired!",gr->gib_pos().x,gr->gib_pos().y);
			message_t::get_instance()->add_message(translator::translate("Orphan signal during loading!"),os1->gib_pos().gib_2d(),message_t::problems);
		}

		// remove second signal from list
		if(os2) {
			signale.remove(os2);
		}

		// now we should have a pair of signals ... or something was very wrong
		grund_t *new_signal_gr=NULL;
		uint8 type = roadsign_besch_t::SIGN_SIGNAL;
		ribi_t::ribi dir=0;

		// now find out about type and direction
		if(os2  &&  !os2->ist_blockiert()) {
			// built the signal here, if possible
			grund_t *tmp=to;
			to = gr;
			gr = tmp;
			if(os2->gib_typ()==ding_t::old_presignal) {
				type = roadsign_besch_t::SIGN_PRE_SIGNAL;
			}
			else if(os2->gib_typ()==ding_t::old_choosesignal) {
				type |= roadsign_besch_t::FREE_ROUTE;
			}
			dir = os2->get_dir();
			directions = 1;
		}
		else {
			// gr is already the first choice
			// so we just have to determine the type
			if(os1->gib_typ()==ding_t::old_presignal) {
				type = roadsign_besch_t::SIGN_PRE_SIGNAL;
			}
			else if(os1->gib_typ()==ding_t::old_choosesignal) {
				type |= roadsign_besch_t::FREE_ROUTE;
			}
		}
		// take care of one way
		if(!os1->ist_blockiert()) {
			directions ++;
			dir |= os1->get_dir();
		}

		// now check where we can built best
		if(ribi_t::is_twoway(gr->gib_weg(wt)->gib_ribi_unmasked())) {
			new_signal_gr = gr;
		}
		if((new_signal_gr==NULL  ||  !os1->ist_blockiert())  &&  to  &&  ribi_t::is_twoway(to->gib_weg(wt)->gib_ribi_unmasked())) {
			new_signal_gr = to;
		}
		if(directions==2  &&  new_signal_gr) {
			dir = new_signal_gr->gib_weg(wt)->gib_ribi_unmasked();
		}

		// found a suitable location, ribi, signal type => construct
		if(new_signal_gr  &&  dir!=0) {
			const roadsign_besch_t *sb=roadsign_t::roadsign_search(type,wt,0);
			if(sb!=NULL) {
				signal_t *sig = new signal_t(welt,new_signal_gr->gib_besitzer(),new_signal_gr->gib_pos(),dir,sb);
				new_signal_gr->obj_pri_add(sig,0);
				sig->laden_abschliessen();	// to make them visible
				new_signal_gr->gib_weg(wt)->count_sign();
//DBG_MESSAGE("old_blockmanager::laden_abschliessen()","signal restored at %i,%i with dir %i",gr->gib_pos().x,gr->gib_pos().y,dir);
			}
			else {
				dbg->error("old_blockmanager_t::laden_abschliessen()","could not restore old signal near (%i,%i), dir=%i",gr->gib_pos().x,gr->gib_pos().y,dir);
				sprintf(buf,err_text,os1->gib_pos().x,os1->gib_pos().y);
				message_t::get_instance()->add_message(buf,os1->gib_pos().gib_2d(),message_t::problems);
				failure++;
			}
		}
		else {
			dbg->warning("old_blockmanager_t::laden_abschliessen()","could not restore old signal near (%i,%i), dir=%i",gr->gib_pos().x,gr->gib_pos().y,dir);
			sprintf(buf,err_text,os1->gib_pos().x,os1->gib_pos().y);
			message_t::get_instance()->add_message(buf,os1->gib_pos().gib_2d(),message_t::problems);
			failure ++;
		}

		os1->setze_pos(koord3d::invalid);
		delete os1;
		if(os2) {
			os2->setze_pos(koord3d::invalid);
			delete os2;
		}
	}
	dbg->warning("old_blockmanager_t::laden_abschliessen()","failed on %d signal pairs.",failure);
}
