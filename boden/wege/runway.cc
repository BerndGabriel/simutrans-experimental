/*
 * Runaways for Simutrans
 *
 * Revised January 2001
 * Hj. Malthaner
 */

#include "../../simworld.h"
#include "../../bauer/wegbauer.h"
#include "../../descriptor/way_desc.h"
#include "../../dataobj/loadsave.h"

#include "runway.h"

const way_desc_t *runway_t::default_runway=NULL;


runway_t::runway_t() : schiene_t()
{
	set_desc(default_runway);
}


runway_t::runway_t(loadsave_t *file) : schiene_t()
{
	rdwr(file);
}


void runway_t::rdwr(loadsave_t *file)
{
	xml_tag_t t( file, "runway_t" );

	weg_t::rdwr(file);

	if(file->is_saving()) {
		const char *s = get_desc()->get_name();
		file->rdwr_str(s);
	}
	else {
		char bname[128];
		file->rdwr_str(bname, lengthof(bname));
		const way_desc_t *desc = way_builder_t::get_desc(bname);
		int old_max_speed=get_max_speed();
		if(desc==NULL) {
			desc = way_builder_t::weg_search(air_wt,old_max_speed>0 ? old_max_speed : 20, 0, (systemtype_t)(old_max_speed>250) );
			if(desc==NULL) {
				desc = default_runway;
				welt->add_missing_paks( bname, karte_t::MISSING_WAY );
			}
			dbg->warning("runway_t::rdwr()", "Unknown runway %s replaced by %s (old_max_speed %i)", bname, desc->get_name(), old_max_speed );
		}
		if(old_max_speed>0) {
			set_max_speed(old_max_speed);
		}
		set_desc(desc);
	}
}

/**
* True, if there are no reservations of other convoys scheduled for the given period.
*/
bool runway_t::can_schedule_reservation(const reservation_schedule_item_t& item) const
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
*/
void runway_t::schedule_reservation(const reservation_schedule_item_t & item)
{
	// iterate through complete schedule to
	// - remove existing items of the convoy, 
	// - remove items of stale convoys and 
	// - insert given item before the first item scheduled for a later time.
	reservation_schedule_t::iterator i = reservations.begin();

	// loop part 1: (terminates at end() or the first *i scheduled for a later time than item)
	while (i != reservations.end() && *i < item)
		if (i->convoy == item.convoy || !i->convoy.is_bound())
			// remove previous or stale reservation
			i = reservations.erase(i);
		else
			++i;

	// insert item before *i (appends if i == end()).
	i = reservations.insert(i, item);
	++i;

	// loop part 2: continue loop until end()
	while (i != reservations.end())
		if (i->convoy == item.convoy || !i->convoy.is_bound())
			// remove previous or stale reservation
			i = reservations.erase(i);
		else
			++i;
}

/**
* Get reservation of given convoy.
* @author B.Gabriel
*/
const reservation_schedule_item_t * runway_t::get_reservation_of(const convoihandle_t & convoy)
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
void runway_t::remove_reservations(const convoihandle_t & convoy)
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

bool runway_t::can_reserve(convoihandle_t c) const
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
				// c's reservation. c can reserve.
				break;

			if (now < i->arrival)
				// first entry in the future. c can reserve.
				break;

			if (now < i->departure)
				// another convoy's reservation at now. c must not reserve.
				return false;
		}
	}
	return true;
}

bool runway_t::reserve(convoihandle_t c, ribi_t::ribi dir)
{
	if (can_reserve(c)) {
		reserved = c;

		// remove from schedule
		remove_reservations(c);

		/* for threeway and fourway switches we may need to alter graphic, if
		* direction is a diagonal (i.e. on the switching part)
		* and there are switching graphics
		*/
		if (ribi_t::is_threeway(get_ribi_unmasked()) && ribi_t::is_bend(dir) && get_desc()->has_switch_image()) {
			mark_image_dirty(get_image(), 0);
			mark_image_dirty(get_front_image(), 0);
			set_images(image_switch, get_ribi_unmasked(), is_snow(), (dir == ribi_t::northeast || dir == ribi_t::southwest));
			set_flag(obj_t::dirty);
		}
		if (schiene_t::show_reservations) {
			set_flag(obj_t::dirty);
		}
		return true;
	}
	// reserve anyway ...
	return false;
}

bool runway_t::unreserve(convoihandle_t c)
{
	// is this tile reserved by us?
	remove_reservations(c);
	if (reserved.is_bound() && reserved == c) {
		reserved = convoihandle_t();
		if (schiene_t::show_reservations) {
			set_flag(obj_t::dirty);
		}
		return true;
	}
	return false;
}
