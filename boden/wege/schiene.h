/*
 * Copyright (c) 1997 - 2001 Hansjörg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#ifndef boden_wege_schiene_h
#define boden_wege_schiene_h


#include "weg.h"
#include "../../convoihandle_t.h"

class vehicle_t;

//BG, 03-Oct-2017: reservation scheduling avoids air traffic congestion.
// air_vehicle::get_cost() penalises use of reserved runways with high costs. 
// Thus alterative runways are preferred.

#include "../../tpl/slist_tpl.h"

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
*/
typedef slist_tpl<reservation_schedule_item_t> reservation_schedule_t;

//BG, 03-Oct-2017: end of insertions for reservation scheduling avoids air traffic congestion.

/**
 * Class for Rails in Simutrans.
 * Trains can run over rails.
 * Every rail belongs to a section block
 *
 * @author Hj. Malthaner
 */
class schiene_t : public weg_t
{
private:
	/**
	* Bound when this block was successfully reserved by the convoi
	* @author prissi
	*/
	convoihandle_t reserved;

//BG, 03-Oct-2017: reservation scheduling avoids air traffic congestion.
// Reservation scheduling is added to schiene_t to encourage using it for
// fair scheduling (first come, first serve) in railway nets as well.

	reservation_schedule_t reservations;

//BG, 03-Oct-2017: end of insertions for reservation scheduling avoids air traffic congestion.
public:
	static const way_desc_t *default_schiene;

	static bool show_reservations;

	/**
	* File loading constructor.
	* @author Hj. Malthaner
	*/
	schiene_t(loadsave_t *file);

	schiene_t();

	virtual waytype_t get_waytype() const {return track_wt;}

	/**
	* @return additional info is reservation!
	* @author prissi
	*/
	void info(cbuffer_t & buf) const;

//BG, 03-Oct-2017: reservation scheduling avoids air traffic congestion.

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

//BG, 03-Oct-2017: end of insertions for reservation scheduling avoids air traffic congestion.

	/**
	* true, if this rail can be reserved
	* @author prissi
	*/
	bool can_reserve(convoihandle_t c) const;

	/**
	* true, if this rail can be reserved
	* @author prissi
	*/
	bool is_reserved() const { return reserved.is_bound(); }

	/**
	* true, then this rail was reserved
	* @author prissi
	*/
	bool reserve(convoihandle_t c, ribi_t::ribi dir);

	/**
	* releases previous reservation
	* @author prissi
	*/
	bool unreserve( convoihandle_t c);

	/**
	* releases previous reservation
	* @author prissi
	*/
	bool unreserve( vehicle_t *);

	/* called before deletion;
	 * last chance to unreserve tiles ...
	 */
	virtual void cleanup(player_t *player);

	/**
	* gets the related convoi
	* @author prissi
	*/
	convoihandle_t get_reserved_convoi() const {return reserved;}

	void rdwr(loadsave_t *file);

	/**
	 * if a function return here a value with TRANSPARENT_FLAGS set
	 * then a transparent outline with the color form the lower 8 Bit is drawn
	 * @author kierongreen
	 */
	virtual FLAGGED_PIXVAL get_outline_colour() const { return (show_reservations  &&  reserved.is_bound()) ? TRANSPARENT75_FLAG | OUTLINE_FLAG | color_idx_to_rgb(COL_RED) : 0;}

	/*
	 * to show reservations if needed
	 */
	virtual image_id get_outline_image() const { return weg_t::get_image(); }
};


template<> inline schiene_t* obj_cast<schiene_t>(obj_t* const d)
{
	return dynamic_cast<schiene_t*>(d);
}

#endif
