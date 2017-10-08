/*
 * Rails for Simutrans
 *
 * Revised January 2001
 * Hj. Malthaner
 */

#include <stdio.h>

#include "../../simconvoi.h"
#include "../../simworld.h"
#include "../../vehicle/simvehicle.h"

#include "../../dataobj/loadsave.h"
#include "../../dataobj/translator.h"

#include "../../utils/cbuffer_t.h"

#include "../../descriptor/way_desc.h"
#include "../../bauer/wegbauer.h"

#include "schiene.h"

const way_desc_t *schiene_t::default_schiene=NULL;
bool schiene_t::show_reservations = false;


schiene_t::schiene_t() : weg_t()
{
	reserved = convoihandle_t();
	set_desc(schiene_t::default_schiene);
}


schiene_t::schiene_t(loadsave_t *file) : weg_t()
{
	reserved = convoihandle_t();
	rdwr(file);
}


void schiene_t::cleanup(player_t *)
{
	// removes reservation
	if(reserved.is_bound()) {
		set_ribi(ribi_t::none);
		reserved->suche_neue_route();
	}
}


void schiene_t::info(cbuffer_t & buf) const
{
	weg_t::info(buf);

	if(reserved.is_bound()) {
		const char* reserve_text = translator::translate("\nis reserved by:");
		// ignore linebreak
		if (reserve_text[0] == '\n') {
			reserve_text++;
		}
		buf.append(reserve_text);
		buf.append(reserved->get_name());
		buf.append("\n");
#ifdef DEBUG_PBS
		reserved->open_info_window();
#endif
	}
}

/**
* True, if there are no reservations of other convoys scheduled for the given period.
* @author B.Gabriel
*/
bool schiene_t::can_schedule_reservation(const reservation_schedule_item_t& item) const
{
	for (reservation_schedule_t::const_iterator i = reservations.begin(); i != reservations.end(); ++i)
	{
		if (item.convoy == i->convoy || !i->convoy.is_bound())
			// ignore my own and stale reservations
			continue;

		if (item.departure <= i->arrival)
		{
			// can insert before this reservation
			return true;
		}
		if (item.arrival < i->departure)
		{
			// intersects this reservation
			return false;
		}
	}
	return true;
}

/**
* Schedule reservation no matter if there are concurrent reservations.
* All other reservations of the convoy for this way are removed before the new reservation is inserted.
* @author B.Gabriel
*/
void schiene_t::schedule_reservation(const reservation_schedule_item_t & item)
{
	remove_reservations(item.convoy);
	for (reservation_schedule_t::iterator i = reservations.begin(); i != reservations.end(); ++i)
	{
		if (item < *i)
		{
			// insert before this item
			i = reservations.insert(i, item);
			return;
		}
	}
	reservations.append(item);
}

/**
* Get reservation of given convoy.
* @author B.Gabriel
*/
const reservation_schedule_item_t * schiene_t::get_reservation_of(const convoihandle_t & convoy)
{
	if (!reservations.empty())
		for (reservation_schedule_t::iterator i = reservations.begin(); i != reservations.end(); ++i)
			if (i->convoy == convoy)
				return &*i;
	return NULL;
}

/**
* Remove all reservations of given convoy.
* @author B.Gabriel
*/
void schiene_t::remove_reservations(const convoihandle_t & convoy)
{
	if (!reservations.empty())
		for (reservation_schedule_t::iterator i = reservations.begin(); i != reservations.end(); )
		{
			if (i->convoy == convoy || !i->convoy.is_bound())
				// remove previous or stale reservation
				i = reservations.erase(i);
			else
				++i;
		}
}

bool schiene_t::can_reserve(convoihandle_t c) const
{
	if (reserved.is_bound())
		return c == reserved;

	if (!reservations.empty())
	{
		// c can reserve only, if c is late or is scheduled for now or no other convoy is scheduled for now.

		uint32 now = welt->get_ticks();
		for (reservation_schedule_t::const_iterator i = reservations.begin(); i != reservations.end(); ++i)
		{
			if (!i->convoy.is_bound())
				// ignore reservations of stale convoys
				continue;

			if (i->convoy == c)
				// c's reservation. c may reserve, if free.
				break;

			if (now < i->arrival)
				// first entry in the future. c may reserve, if free.
				break;

			if (now < i->departure)
				// another convoy's reservation at now. c must not reserve.
				return false;
		}
	}
	return true;
}

/**
 * true, if this rail can be reserved
 * @author prissi
 */
bool schiene_t::reserve(convoihandle_t c, ribi_t::ribi dir  )
{
	if(can_reserve(c)) {
		reserved = c;

		// remove from schedule
		remove_reservations(c);

		/* for threeway and fourway switches we may need to alter graphic, if
		 * direction is a diagonal (i.e. on the switching part)
		 * and there are switching graphics
		 */
		if(  ribi_t::is_threeway(get_ribi_unmasked())  &&  ribi_t::is_bend(dir)  &&  get_desc()->has_switch_image()  ) {
			mark_image_dirty( get_image(), 0 );
			mark_image_dirty( get_front_image(), 0 );
			set_images(image_switch, get_ribi_unmasked(), is_snow(), (dir==ribi_t::northeast  ||  dir==ribi_t::southwest) );
			set_flag( obj_t::dirty );
		}
		if(schiene_t::show_reservations) {
			set_flag( obj_t::dirty );
		}
		return true;
	}
	// reserve anyway ...
	return false;
}


/**
* releases previous reservation
* only true, if there was something to release
* @author prissi
*/
bool schiene_t::unreserve(convoihandle_t c)
{
	// is this tile reserved by us?
	remove_reservations(c);
	if(reserved.is_bound()  &&  reserved==c) {
		reserved = convoihandle_t();
		if(schiene_t::show_reservations) {
			set_flag( obj_t::dirty );
		}
		return true;
	}
	return false;
}




/**
* releases previous reservation
* @author prissi
*/
bool schiene_t::unreserve(vehicle_t *v)
{
	// is this tile empty?
	if(!reserved.is_bound()) {
		return true;
	}
	if (reserved.get_rep() == v->get_convoi()) {
		reserved = convoihandle_t();
		if(schiene_t::show_reservations) {
			set_flag( obj_t::dirty );
		}
		return true;
	}
	return false;
}


void schiene_t::rdwr(loadsave_t *file)
{
	xml_tag_t t( file, "schiene_t" );

	weg_t::rdwr(file);

	if(file->get_version()<99008) {
		sint32 blocknr=-1;
		file->rdwr_long(blocknr);
	}

	if(file->get_version()<89000) {
		uint8 dummy;
		file->rdwr_byte(dummy);
		set_electrify(dummy);
	}

	if(file->is_saving()) {
		const char *s = get_desc()->get_name();
		file->rdwr_str(s);
	}
	else {
		char bname[128];
		file->rdwr_str(bname, lengthof(bname));

		int old_max_speed=get_max_speed();
		const way_desc_t *desc = way_builder_t::get_desc(bname);
		if(desc==NULL) {
			int old_max_speed=get_max_speed();
			desc = way_builder_t::get_desc(translator::compatibility_name(bname));
			if(desc==NULL) {
				desc = default_schiene;
				welt->add_missing_paks( bname, karte_t::MISSING_WAY );
			}
			dbg->warning("schiene_t::rdwr()", "Unknown rail %s replaced by %s (old_max_speed %i)", bname, desc->get_name(), old_max_speed );
		}
		set_desc(desc);
		if(old_max_speed>0) {
			set_max_speed(old_max_speed);
		}
		//DBG_MESSAGE("schiene_t::rdwr","track %s at (%i,%i) max_speed %i",bname,get_pos().x,get_pos().y,old_max_speed);
	}
}
