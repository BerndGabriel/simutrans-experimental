/*
 * Copyright (c) 1997 - 2001 Hansjörg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#ifndef boden_wege_runway_h
#define boden_wege_runway_h


#include "schiene.h"
#include "../../tpl/vector_tpl.h"

struct reservation_schedule_item_t
{
	convoihandle_t	convoy;
	uint32			arrival;	// in ticks
	uint32			departure;	// in ticks

	reservation_schedule_item_t()
		: arrival(0)
		, departure(0)
	{}

	reservation_schedule_item_t(
		const convoihandle_t& convoy,
		uint32			arrival,
		uint32			departure)
		: convoy(convoy)
		, arrival(arrival)
		, departure(departure)
	{}

	bool operator < (const reservation_schedule_item_t& that) const
	{
		if (arrival < that.arrival)
			return true;
		if (arrival == that.arrival)
			return departure < that.departure;
		return false;
	}
};

/**
* A list of convoys that intend to use the way at a given time period.
* The list is build by schedule_reservation() sorted by operator < of the items.
*
* Reservation scheduling avoids air traffic congestion:
* air_vehicle::get_cost() penalises use of reserved runways with high costs.
* Thus alterative runways are preferred.
*/
class reservation_schedule_t : public vector_tpl<reservation_schedule_item_t>
{
public:

	/** insert data at a certain pos */
	iterator insert(const iterator& i, const reservation_schedule_item_t& elem)
	{
		uint32 pos = i - begin();
		insert_at(pos, elem);
		return begin() + pos;
	}
};

//BG, 03-Oct-2017: end of insertions for reservation scheduling avoids air traffic congestion.


/**
 * Class for aiport runaway in Simutrans.
 * speed >250 are for take of (maybe rather use system type in next release?)
 *
 * @author Hj. Malthaner
 */
class runway_t : public schiene_t
{
	reservation_schedule_t reservations;
public:
	static const way_desc_t *default_runway;

	/**
	 * File loading constructor.
	 *
	 * @author Hj. Malthaner
	 */
	runway_t(loadsave_t *file);

	runway_t();

	inline waytype_t get_waytype() const {return air_wt;}

	void rdwr(loadsave_t *file);

	/**
	* True, if there are no reservations of other convoys scheduled for the given period.
	*/
	bool can_schedule_reservation(const reservation_schedule_item_t & item) const;

	/**
	* Schedule reservation no matter if there are concurrent reservations.
	* All other reservations of the convoy for this way are removed before the new reservation is inserted.
	*/
	void schedule_reservation(const reservation_schedule_item_t & item);

	/**
	* Get reservation of given convoy:
	*/
	const reservation_schedule_item_t* get_reservation_of(const convoihandle_t& convoy);

	/**
	* Remove all reservations of this convoy.
	*/
	void remove_reservations(const convoihandle_t& convoy);

	/**
	* Get reservation schedule.
	*/
	const reservation_schedule_t& get_reservations() const { return reservations; }

	/**
	* True, if this rail can be reserved.
	*/
	bool can_reserve(convoihandle_t c) const;

	/**
	* True, if this rail has been reserved.
	*/
	bool reserve(convoihandle_t c, ribi_t::ribi dir);

	/**
	* Releases all reservations of given convoy.
	*/
	bool unreserve(convoihandle_t c);
};

#endif
