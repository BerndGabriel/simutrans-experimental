/*
 * Copyright (c) 1997 - 2001 Hj. Malthaner
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

/*
 * zentrale Datenstruktur von Simutrans
 * von Hj. Malthaner 1998
 */

#ifndef simworld_h
#define simworld_h

#include "simconst.h"
#include "simtypes.h"
#include "simunits.h"

#include "convoihandle_t.h"
#include "halthandle_t.h"

#include "tpl/weighted_vector_tpl.h"
#include "tpl/vector_tpl.h"
#include "tpl/slist_tpl.h"

#include "dataobj/marker.h"
#include "dataobj/einstellungen.h"
#include "dataobj/pwd_hash.h"

#include "simplan.h"

#include "simdebug.h"
#ifdef DEBUG_SIMRAND_CALLS
#include "utils/cbuffer_t.h"
#include "tpl/fixed_list_tpl.h"
#endif

struct event_t;
struct sound_info;
class stadt_t;
class fabrik_t;
class gebaeude_t;
class zeiger_t;
class grund_t;
class planquadrat_t;
class karte_ansicht_t;
class sync_steppable;
class werkzeug_t;
class scenario_t;
class message_t;
class weg_besch_t;
class tunnel_besch_t;
class network_world_command_t;
class ware_besch_t;
class memory_rw_t;

struct checklist_t
{
	uint32 random_seed;
	uint16 halt_entry;
	uint16 line_entry;
	uint16 convoy_entry;

	uint32 industry_density_proportion;
	uint32 actual_industry_density;
	uint32 traffic;

	checklist_t() : random_seed(0), halt_entry(0), line_entry(0), convoy_entry(0), industry_density_proportion(0), actual_industry_density(0), traffic(0) { }
	checklist_t(uint32 _random_seed, uint16 _halt_entry, uint16 _line_entry, uint16 _convoy_entry, uint32 _industry_denisty_proportion, uint32 _actual_industry_density, uint32 _traffic)
		: random_seed(_random_seed), halt_entry(_halt_entry), line_entry(_line_entry), convoy_entry(_convoy_entry), industry_density_proportion(_industry_denisty_proportion), actual_industry_density(_actual_industry_density), traffic(_traffic) { }

	bool operator == (const checklist_t &other) const
	{
		return ( random_seed==other.random_seed && halt_entry==other.halt_entry && line_entry==other.line_entry && convoy_entry==other.convoy_entry && industry_density_proportion == other.industry_density_proportion && actual_industry_density == other.actual_industry_density);
	}
	bool operator != (const checklist_t &other) const { return !( (*this)==other ); }

	void rdwr(memory_rw_t *buffer);
	int print(char *buffer, const char *entity) const;
};


/**
 * Die Karte ist der zentrale Bestandteil der Simulation. Sie
 * speichert alle Daten und Objekte.
 *
 * @author Hj. Malthaner
 */
class karte_t
{
public:

#ifdef DEBUG_SIMRAND_CALLS
	static bool print_randoms;
	static int random_calls;
#endif
	/**
	* Hoehe eines Punktes der Karte mit "perlin noise"
	*
	* @param frequency in 0..1.0 roughness, the higher the rougher
	* @param amplitude in 0..160.0 top height of mountains, may not exceed 160.0!!!
	* @author Hj. Malthaner
	*/

	static sint32 perlin_hoehe(settings_t const*, koord pos, koord const size, const sint32 map_size);

	enum player_cost {
		WORLD_CITICENS=0,// total people
		WORLD_GROWTH,	// growth (just for convenience)
		WORLD_TOWNS,	// number of all cities
		WORLD_FACTORIES,	// number of all consuming only factories
		WORLD_CONVOIS,	// total number of convois
		WORLD_CITYCARS,	// Number of private car trips
		WORLD_PAS_RATIO,	// percentage of passengers that started successful
		WORLD_PAS_GENERATED,	// total number generated
		WORLD_MAIL_RATIO,	// percentage of mail that started successful
		WORLD_MAIL_GENERATED,	// all letters generated
		WORLD_GOODS_RATIO, // ratio of chain completeness
		WORLD_TRANSPORTED_GOODS, // all transported goods
		MAX_WORLD_COST
	};

	#define MAX_WORLD_HISTORY_YEARS  (12) // number of years to keep history
	#define MAX_WORLD_HISTORY_MONTHS  (12) // number of months to keep history


	bool get_is_shutting_down() const { return is_shutting_down; }

	// City cars that are not assigned to a particular city are stored in this list.
	// @author:jamespetts
	slist_tpl<stadtauto_t *> unassigned_cars;
	
	void add_unassigned_car(stadtauto_t* car) { unassigned_cars.append(car); } 

	enum { NORMAL=0, PAUSE_FLAG = 0x01, FAST_FORWARD=0x02, FIX_RATIO=0x04 };

	/* Missing things during loading:
	 * factories, vehicles, roadsigns or catenary may be severe
	 */
	enum missing_level_t { NOT_MISSING=0, MISSING_FACTORY=1, MISSING_VEHICLE=2, MISSING_SIGN=3, MISSING_WAYOBJ=4, MISSING_ERROR=4, MISSING_BRIDGE, MISSING_BUILDING, MISSING_WAY };

private:
	settings_t settings;

	// aus performancegruenden werden einige Einstellungen local gecached
	sint16 cached_groesse_gitter_x;
	sint16 cached_groesse_gitter_y;
	// diese Werte sind um eins kleiner als die Werte fuer das Gitter
	sint16 cached_groesse_karte_x;
	sint16 cached_groesse_karte_y;
	// maximum size for waitng bars etc.
	int cached_groesse_max;

	// all cursor interaction goes via this function
	// it will call save_mouse_funk first with init, then with the position and with exit, when another tool is selected without click
	// see simwerkz.cc for practical examples of such functions
	werkzeug_t *werkzeug[MAX_PLAYER_COUNT];

	// Whether the map is currently being destroyed. 
	// Useful to prevent access violations if objects with
	// references to other objects that are destroyed first
	// reference members of those objects in their destructors.
	bool is_shutting_down; 

	/**
	 * redraw whole map
	 */
	bool dirty;

	/**
	 * fuer softes scrolling
	 */
	sint16 x_off, y_off;

	/* current position */
	koord ij_off;

	/* this is the current offset for getting from tile to screen */
	koord ansicht_ij_off;

	/**
	 * Position of the mouse pointer (internal)
	 * @author Hj. Malthaner
	 */
	sint32 mi, mj;

	/* time when last mouse moved to check for ambient sound events */
	uint32 mouse_rest_time;
	uint32 sound_wait_time;	// waiting time before next event

	/**
	 * If this is true, the map will not be scrolled
	 * on right-drag
	 * @author Hj. Malthaner
	 */
	bool scroll_lock;

	// if true, this map cannot be saved
	bool nosave;
	bool nosave_warning;

	/*
	* the current convoi to follow
	* @author prissi
	*/
	convoihandle_t follow_convoi;

	/**
	 * water level height
	 * @author Hj. Malthaner
	 */
	sint8 grundwasser;

	/**
	 * current snow height (might change during the game)
	 * @author prissi
	 */
	sint16 snowline;

	// changes the snowline height (for the seasons)
	// returns true if a change is needed
	// @author prissi
	bool recalc_snowline();

	// >0 means a season change is needed
	int pending_season_change;

	// recalculates sleep time etc.
	void update_frame_sleep_time(long delta_t);

	/**
	 * table for fast conversion from height to climate
	 * @author prissi
	 */
	uint8 height_to_climate[32];

	zeiger_t *zeiger;

	slist_tpl<sync_steppable *> sync_add_list;	// these objects are move to the sync_list (but before next sync step, so they do not interfere!)
#ifndef SYNC_VECTOR
	slist_tpl<sync_steppable *> sync_list;
#else
	vector_tpl<sync_steppable *> sync_list;
#endif
	slist_tpl<sync_steppable *> sync_remove_list;

	vector_tpl<convoihandle_t> convoi_array;

	vector_tpl<fabrik_t *> fab_list;

	// Stores a list of goods produced by factories currently in the game;
	vector_tpl<const ware_besch_t*> goods_in_game;

	weighted_vector_tpl<gebaeude_t *> ausflugsziele;

	slist_tpl<koord> labels;

	weighted_vector_tpl<stadt_t*> stadt;

	sint64 last_month_bev;

	// the recorded history so far
	sint64 finance_history_year[MAX_WORLD_HISTORY_YEARS][MAX_WORLD_COST];
	sint64 finance_history_month[MAX_WORLD_HISTORY_MONTHS][MAX_WORLD_COST];
	
	// word record of speed ...
	class speed_record_t {
	public:
		convoihandle_t cnv;
		sint32	speed;
		koord	pos;
		spieler_t *besitzer;
		sint32 year_month;

		speed_record_t() : cnv(), speed(0), pos(koord::invalid), besitzer(NULL), year_month(0) {}
	};

	speed_record_t max_rail_speed;
	speed_record_t max_monorail_speed;
	speed_record_t max_maglev_speed;
	speed_record_t max_narrowgauge_speed;
	speed_record_t max_road_speed;
	speed_record_t max_ship_speed;
	speed_record_t max_air_speed;

	karte_ansicht_t *view;

	/**
	 * Raise tile (x,y): height of each corner is given
	 */
	bool can_raise_to(sint16 x, sint16 y, bool keep_water, sint8 hsw, sint8 hse, sint8 hne, sint8 hnw, uint8 ctest=15) const;
	int  raise_to(sint16 x, sint16 y, sint8 hsw, sint8 hse, sint8 hne, sint8 hnw);
	/**
	 * Raise grid point (x,y), used during map creation/enlargement
	 */
	int  raise_to(sint16 x, sint16 y, sint8 h, bool set_slopes);

	/**
	 * Lower tile (x,y): height of each corner is given
	 */
	bool can_lower_to(sint16 x, sint16 y, sint8 hsw, sint8 hse, sint8 hne, sint8 hnw, uint8 ctest=15) const;
	int  lower_to(sint16 x, sint16 y, sint8 hsw, sint8 hse, sint8 hne, sint8 hnw);
	/**
	 * Lower grid point (x,y), used during map creation/enlargement
	 */
	int  lower_to(sint16 x, sint16 y, sint8 h, bool set_slopes);

	/**
	 * Die fraktale Erzeugung der Karte ist nicht perfekt.
	 * cleanup_karte() beseitigt etwaige Fehler.
	 * @author Hj. Malthaner
	 */
	void cleanup_karte( int xoff, int yoff );

	void blick_aendern(event_t *ev);
	void bewege_zeiger(const event_t *ev);
	void interactive_event(event_t &ev);

	planquadrat_t *plan;

	sint8 *grid_hgts;

	marker_t marker;

	/**
	 * The players of the game
	 * @author Hj. Malthaner
	 */
	spieler_t *spieler[MAX_PLAYER_COUNT];   // Human player has index 0 (zero)
	spieler_t *active_player;
	uint8 active_player_nr;

	/**
	 * locally store password hashes
	 * will be used after reconnect to a server
	 */
	pwd_hash_t player_password_hash[MAX_PLAYER_COUNT];

	/*
	 * counter for schedules
	 * if a new schedule is active, this counter will increment
	 * stations check this counter and will reroute their goods if changed
	 * @author prissi
	 */
	uint8 schedule_counter;

	/**
	 * The time in ms (milliseconds)
	 * @author Hj. Malthaner
	 */
	sint64 ticks;		      // ms since creation
	sint64 last_step_ticks; // ticks counter at last steps
	sint64 next_month_ticks;	// from now on is next month

	// default time stretching factor
	uint32 time_multiplier;

	uint8 step_mode;

	// Variables used in interactive()
	uint32 sync_steps;
#define LAST_CHECKLISTS_COUNT 64
	checklist_t last_checklists[LAST_CHECKLISTS_COUNT];
#define LCHKLST(x) (last_checklists[(x) % LAST_CHECKLISTS_COUNT])
	uint8  network_frame_count;
	uint32 fix_ratio_frame_time; // set in reset_timer()

	/**
	 * For performance comparison
	 * @author Hj. Malthaner
	 */
	uint32 realFPS;
	uint32 simloops;

	// to calculate the fps and the simloops
	uint32 last_frame_ms[32];
	uint32 last_step_nr[32];
	uint8 last_frame_idx;
	uint32 last_interaction;	// ms, when the last time events were handled
	uint32 last_step_time;	// ms, when the last step was done
	uint32 next_step_time;	// ms, when the next step is to be done
//	sint32 time_budget;	// takes care of how many ms I am lagging or are in front of
	uint32 idle_time;

	sint32 current_month;  // monat+12*jahr
	sint32 letzter_monat;  // Absoluter Monat 0..12
	sint32 letztes_jahr;   // Absolutes Jahr

	uint8 season;	// current season

	long steps;          // number of steps since creation
	bool is_sound;       // flag, that now no sound will play
	bool finish_loop;    // flag for ending simutrans (true -> end simutrans)

	// may change due to timeline
	const weg_besch_t *city_road;

	// Data for maintaining industry density even
	// after industries close
	// @author: jamespetts
	uint32 industry_density_proportion;
	uint32 actual_industry_density;

	// what game objectives
	scenario_t *scenario;

	message_t *msg;

	sint32 average_speed[8];

	uint32 tile_counter;

	// to identify different stages of the same game
	uint32 map_counter;

	// recalculated speed boni for different vehicles
	void recalc_average_speed();

	void neuer_monat();      // monthly actions
	void neues_jahr();       // yearly actions

	/**
	 * internal saving method
	 * @author Hj. Malthaner
	 */
	void speichern(loadsave_t *file,bool silent);

	/**
	 * internal loading method
	 * @author Hj. Malthaner
	 */
	void laden(loadsave_t *file);

	/**
	 * entfernt alle objecte, loescht alle datenstrukturen
	 * gibt allen erreichbaren speicher frei
	 * @author Hj. Malthaner
	 */
	void destroy();

	// restores history for older savegames
	void restore_history();

	/*
	 * Will create rivers.
	 */
	void create_rivers(sint16 number);

	/**
	 * Distribute groundobjs and cities on the map but not
	 * in the rectangle from (0,0) till (old_x, old_y).
	 * It's now an extra function so we don't need the code twice.
	 * @auther Gerd Wachsmuth
	 */
	void distribute_groundobjs_cities(settings_t const * const set, sint16 old_x, sint16 old_y);

	// Used for detecting whether paths/connexions are stale.
	// @author: jamespetts
	uint16 base_pathing_counter;

	sint32 citycar_speed_average;

	void set_citycar_speed_average();

	bool recheck_road_connexions;

	/**
	 * Speed per tile in *tenths* of
	 * minutes.
	 * @author: jamespetts, April 2010
	 */
	uint16 generic_road_speed_city;
	uint16 generic_road_speed_intercity;

	uint32 max_road_check_depth;

	// The last time when a server announce was performed (in ms)
	uint32 server_last_announce_time;

public:
	// Announce server and current state to listserver
	// Single argument specifies what information should be announced
	// or offline (the latter only in cases where it is shutting down)
	void announce_server(int status);

	// The month in which the next city generated will update its private car 
	// routes if an update is needed. This spreads the computational load over
	// a year instead of forcing it all into a month, thus improving 
	// performance.
	// @author: jamespetts, February 2011
	uint8 next_private_car_update_month;

	vector_tpl<fabrik_t*> closed_factories_this_month;

	/* reads height data from 8 or 25 bit bmp or ppm files
	 * @return either pointer to heightfield (use delete [] for it) or NULL
	 */
	static bool get_height_data_from_file( const char *filename, sint8 grundwasser, sint8 *&hfield, sint16 &ww, sint16 &hh, bool update_only_values );

	message_t *get_message() const { return msg; }

	// set to something useful, if there is a total distance != 0 to show in the bar below
	koord3d show_distance;

	/* for warning, when stuff had to be removed/replaced
	 * level must be >=1 (1=factory, 2=vechiles, 3=not so important)
	 * may be refined later
	 */
	void add_missing_paks( const char *name, missing_level_t critical_level );

	/**
	 * Absoluter Monat
	 * @author prissi
	 */
	inline uint32 get_last_month() const { return letzter_monat; }

	// @author hsiegeln
	inline sint32 get_last_year() const { return letztes_jahr; }

	/**
	 * dirty: redraw whole screen
	 * @author Hj. Malthaner
	 */
	void set_dirty() {dirty=true;}
	void set_dirty_zurueck() {dirty=false;}
	bool ist_dirty() const {return dirty;}

	// do the internal accounting
	void buche(sint64 betrag, player_cost type);

	// calculates the various entries
	void update_history();

	scenario_t *get_scenario() const { return scenario; }

	/**
	* Returns the finance history for player
	* @author hsiegeln
	*/
	sint64 get_finance_history_year(int year, int type) const { return finance_history_year[year][type]; }
	sint64 get_finance_history_month(int month, int type) const { return finance_history_month[month][type]; }

	/**
	 * Returns pointer to finance history for player
	 * @author hsiegeln
	 */
	const sint64* get_finance_history_year() const { return *finance_history_year; }
	const sint64* get_finance_history_month() const { return *finance_history_month; }

	// recalcs all map images
	void update_map();

	karte_ansicht_t *get_ansicht() const { return view; }
	void set_ansicht(karte_ansicht_t *v) { view = v; }

	/**
	 * viewpoint in tile koordinates
	 * @author Hj. Malthaner
	 */
	koord get_world_position() const { return ij_off; }

	// fine offset within the viewport tile
	int get_x_off() const {return x_off;}
	int get_y_off() const {return y_off;}

	/**
	 * set center viewport position
	 * @author prissi
	 */
	void change_world_position( koord ij, sint16 x=0, sint16 y=0 );

	// also take height into account
	void change_world_position( koord3d ij );

	// the koordinates between the screen and a tile may have several offset
	// this routine caches them
	void set_ansicht_ij_offset( koord k ) { ansicht_ij_off=k; }
	koord get_ansicht_ij_offset() const { return ansicht_ij_off; }

	/**
	 * If this is true, the map will not be scrolled
	 * on right-drag
	 * @author Hj. Malthaner
	 */
	void set_scroll_lock(bool yesno);

	/* functions for following a convoi on the map
	* give an unbound handle to unset
	*/
	void set_follow_convoi(convoihandle_t cnv) { follow_convoi = cnv; }
	convoihandle_t get_follow_convoi() const { return follow_convoi; }

	settings_t const& get_settings() const { return settings; }
	settings_t&       get_settings()       { return settings; }

	// returns current speed bonus
	sint32 get_average_speed(waytype_t typ) const 
	{ 
		const sint32 return_value = average_speed[ (typ==16 ? 3 : (int)(typ-1)&7 ) ];
		return return_value > 0 ? return_value : 1;
	}

	// speed record management
	sint32 get_record_speed( waytype_t w ) const;
	void notify_record( convoihandle_t cnv, sint32 max_speed, koord pos );

	// time lapse mode ...
	bool is_paused() const { return step_mode&PAUSE_FLAG; }
	void set_pause( bool );	// stops the game with interaction

	bool is_fast_forward() const { return step_mode == FAST_FORWARD; }
	void set_fast_forward(bool ff);

	// (un)pause for network games
	void network_game_set_pause(bool pause_, uint32 syncsteps_);

	zeiger_t * get_zeiger() const { return zeiger; }

	/**
	 * marks an area using the grund_t mark flag
	 * @author prissi
	 */
	void mark_area( const koord3d center, const koord radius, const bool mark ) const;

	/**
	 * Player management here
	 */
	uint8 sp2num(spieler_t *sp);
	spieler_t * get_spieler(uint8 n) const { return spieler[n&15]; }
	spieler_t* get_active_player() const { return active_player; }
	uint8 get_active_player_nr() const { return active_player_nr; }
	void switch_active_player(uint8 nr, bool silent);
	const char *new_spieler( uint8 nr, uint8 type );
	void store_player_password_hash( uint8 player_nr, const pwd_hash_t& hash );
	const pwd_hash_t& get_player_password_hash( uint8 player_nr ) const { return player_password_hash[player_nr]; }
	void clear_player_password_hashes();
	void rdwr_player_password_hashes(loadsave_t *file);

	/**
	 * network safe initiation of new players
	 */
	void call_change_player_tool(uint8 cmd, uint8 player_nr, uint16 param);

	enum change_player_tool_cmds { new_player=1, toggle_freeplay=2 };
	/**
	 * @param exec: if false checks whether execution is allowed
	 *              if true executes tool
	 * @returns whether execution is allowed
	 */
	bool change_player_tool(uint8 cmd, uint8 player_nr, uint16 param, bool public_player_unlocked, bool exec);

	// if a schedule is changed, it will increment the schedule counter
	// every step the haltestelle will check and reroute the goods if needed
	uint8 get_schedule_counter() const { return schedule_counter; }
	void set_schedule_counter();

	// often used, therefore found here
	bool use_timeline() const { return settings.get_use_timeline(); }

	void reset_timer();
	void reset_interaction();
	void step_year();

	// jump one or more months ahead
	// (updating history!)
	void step_month( sint16 months=1 );

	// returns either 0 or the current year*16 + month
	uint16 get_timeline_year_month() const { return settings.get_use_timeline() ? current_month : 0; }

	/**
	* anzahl ticks pro tag in bits
	* @see ticks_per_world_month
	* @author Hj. Malthaner
	*
	* number ticks per day in bits (Babelfish)
	*/

	sint64 ticks_per_world_month_shift; 


	/**
	* anzahl ticks pro MONTH!
	* @author Hj. Malthaner
	*
	* number ticks per MONTH! (Babelfish)
	*/

	sint64 ticks_per_world_month;

	void set_ticks_per_world_month_shift(sint64 bits) {ticks_per_world_month_shift = bits; ticks_per_world_month = (1LL << ticks_per_world_month_shift); }

	/**
	 * Converts speed (yards per tick) into tiles per month
	 *       * A derived quantity:
	 * speed * ticks_per_world_month / yards_per_tile
	 * == speed << ticks_per_world_month_shift  (pak-set dependent, 18 in old games)
	 *          >> yards_per_tile_shift (which is 12 + 8 = 20, see above)
	 * This is hard to do with full generality, because shift operators
	 * take positive numbers!
	 *
	 * @author neroden
	 */
	uint32 speed_to_tiles_per_month(uint32 speed) const
	{
		const int left_shift = (int)(ticks_per_world_month_shift - YARDS_PER_TILE_SHIFT);
		if (left_shift >= 0) {
			return speed << left_shift;
		} else {
			const int right_shift = -left_shift;
			// round to nearest
			return (speed + (1<<(right_shift -1)) ) >> right_shift;
		}
	}

	sint32 get_time_multiplier() const { return time_multiplier; }
	void change_time_multiplier( sint32 delta );

	/** 
	 * calc_adjusted_monthly_figure()
	 *
	 * Quantities defined on a per month base must be adjusted according to the virtual
	 * game speed defined by the ticks per month setting in simuconf.tab.
	 *
	 * NOTICE: Don't confuse, on the one hand, the game speed, represented by the number in the 
	 * lower right hand corner, and, on the other hand, the ticks per month setting. 
	 *
	 * The first increases or decreases the speed of all things that happen in the game by the 
	 * same amount: the whole game is fast forwarded or slowed down.
	 *
	 * The second is very different: it alters the relative scale of the speed of the game, 
	 * on the one hand, against the passing of time on the other. Increasing the speed using 
	 * the ticks per month setting (which requires reducing the number in simuconf.tab) 
	 * increases the number of months/years that pass in comparison to everything else that 
	 * happens in the game.
	 *
	 * Example: 
	 *
	 * suppose that you have a railway between two towns: A and B, and one train on 
	 * that railway. At a speed setting of 1.00 and a ticks per month setting of 18 (the default), 
	 * suppose that your train goes between A and B ten times per month, which takes ten real minutes. 
	 * At a speed setting of 2.00 and the same ticks per month setting, in ten real minutes, the train 
	 * would travel between A and B twenty times, earning twice as much revenue, and costing twice as 
	 * much in maintenance, and two months would pass.
	 * 
	 * However, if, instead of increasing the time to 2.00, you reduced the ticks per month to 17 and 
	 * left the game speed at 1.00, in ten minutes of real time, the train would still travel ten times 
	 * between A and B and earn the same amount of revenue, and cost the same amount in maintenance as 
	 * the first instance, but two game months would pass.
	 *
	 * It should be apparent from that description that the maintenance cost per month needs to be 
	 * scaled with the proportion of months to ticks, since, were it not, lowering the ticks per month 
	 * setting would mean that the fixed maintenance cost (the per month cost) would increase the 
	 * monthly maintenance cost, but not the variable maintenance cost. Since changing the ticks per 
	 * month setting is supposed to be cost-neutral, this cannot happen, so all costs that are 
	 * calculated monthly have to be adjusted to take account of the ticks per month setting in order 
	 * to counteract its effect.
	 * 
	 * James E. Petts
	 *
	 * same adjustment applies to production rates.
	 *
	 * @author: Bernd Gabriel, 14.06.2009
	 */
	sint32 calc_adjusted_monthly_figure(sint32 nominal_monthly_figure) {
		if (ticks_per_world_month_shift >= 18) {
			return (sint32)(nominal_monthly_figure << (ticks_per_world_month_shift - 18l)); 
		} else {
			return (sint32)(nominal_monthly_figure >> (18l - ticks_per_world_month_shift)); 
		}
	}
	sint64 calc_adjusted_monthly_figure(sint64 nominal_monthly_figure) {
		if (ticks_per_world_month_shift >= 18) {
			return nominal_monthly_figure << (ticks_per_world_month_shift - 18ll); 
		} else {
			return nominal_monthly_figure >> (18ll - ticks_per_world_month_shift); 
		}
	}
	uint32 calc_adjusted_monthly_figure(uint32 nominal_monthly_figure) {
		if (ticks_per_world_month_shift >= 18) {
			return (uint32)(nominal_monthly_figure << ((uint32)ticks_per_world_month_shift - 18u)); 
		} else {
			return (uint32)(nominal_monthly_figure >> (18u - (uint32)ticks_per_world_month_shift)); 
		}
	}

	/**
	 * Standard timing conversion
	 * @author: jamespetts
	 */
	sint64 ticks_to_tenths_of_minutes(sint64 ticks) const;

	/**
	 * Finer timing conversion for UI only
	 * @author: jamespetts
	 */
	sint64 ticks_to_seconds(sint64 ticks) const;

	/**
	 * 0=winter, 1=spring, 2=summer, 3=autumn
	 * @author prissi
	 */
	uint8 get_jahreszeit() const { return season; }

	/**
	 * Zeit seit Kartenerzeugung/dem letzen laden in ms
	 * @author Hj. Malthaner
	 *
	 * 	
	 * Time cards since creation / the last load in ms (Google)
	 */
	sint64 get_zeit_ms() const { return ticks; }

	/**
	 * absolute month (count start year zero)
	 * @author prissi
	 */
	uint32 get_current_month() const { return current_month; }

	/**
	 * 1/8th month (0..95) within current year
	 *
	 * Is used to calculate seasonal images.
	 *
	 * Another ticks_bits_per_tag algorithm, I found repeatedly, 
	 * although ticks_bits_per_tag should be private property of karte_t.
	 *
	 * @author: Bernd Gabriel, 14.06.2009
	 */
	int get_yearsteps() { return (int) ((current_month % 12) * 8 + ((ticks >> (ticks_per_world_month_shift-3)) & 7)); }

	// prissi: current city road
	// may change due to timeline
	const weg_besch_t* get_city_road() const { return city_road; }

	/**
	 * Anzahl steps seit Kartenerzeugung
	 * @author Hj. Malthaner
	 *
	 * Number of steps since map production (Babelfish)
	 */
	long get_steps() const { return steps; }

	/**
	 * Idle time. Nur zur Anzeige verwenden!
	 * @author Hj. Malthaner
	 *
	 * Idle time. Use only to the announcement! (Babelfish)
	 */
	uint32 get_schlaf_zeit() const { return idle_time; }

	/**
	 * Anzahl frames in der letzten Sekunde Realzeit
	 * @author prissi
	 *
	 * Number of frames in the last second of real time (Babelfish)
	 */
	uint32 get_realFPS() const { return realFPS; }

	/**
	 * Anzahl Simulationsloops in der letzten Sekunde. Kann sehr ungenau sein!
	 * @author Hj. Malthaner
	 *
	 * Number of simulation loops in the last second. Can be very inaccurate! (Babelfish)
	 */
	uint32 get_simloops() const { return simloops; }

	/**
	* Holt den Grundwasserlevel der Karte
	* @author Hj. Malthaner
	*
	* Gets the groundwater level of the map (Babelfish)
	*/
	sint8 get_grundwasser() const { return grundwasser; }

	/**
	* returns the current snowline height
	* @author prissi
	*/
	sint16 get_snowline() const { return snowline; }

	/**
	* returns the current climate for a given height
	* uses as private lookup table for speed
	* @author prissi
	*/
	climate get_climate(sint16 height) const
	{
		const sint16 h=(height-grundwasser)/Z_TILE_STEP;
		if(h<0) {
			return water_climate;
		} else if(h>=32) {
			return arctic_climate;
		}
		return (climate)height_to_climate[h];
	}

	// set a new tool as current: calls local_set_werkzeug or sends to server
	void set_werkzeug( werkzeug_t *w, spieler_t * sp );
	// set a new tool on our client, calls init
	void local_set_werkzeug( werkzeug_t *w, spieler_t * sp );
	werkzeug_t *get_werkzeug(uint8 nr) const { return werkzeug[nr]; }

	// all stuff concerning map size
	inline int get_groesse_x() const { return cached_groesse_gitter_x; }
	inline int get_groesse_y() const { return cached_groesse_gitter_y; }
	inline int get_groesse_max() const { return cached_groesse_max; }

	inline bool ist_in_kartengrenzen(koord k) const {
		// prissi: since negative values will make the whole result negative, we can use bitwise or
		// faster, since pentiums and other long pipeline processors do not like jumps
		return (k.x|k.y|(cached_groesse_karte_x-k.x)|(cached_groesse_karte_y-k.y))>=0;
		// this is only 67% of the above speed
		//return k.x>=0 &&  k.y>=0  &&  cached_groesse_karte_x>=k.x  &&  cached_groesse_karte_y>=k.y;
	}

	inline bool ist_in_kartengrenzen(sint16 x, sint16 y) const {
	// prissi: since negative values will make the whole result negative, we can use bitwise or
	// faster, since pentiums and other long pipeline processors do not like jumps
		return (x|y|(cached_groesse_karte_x-x)|(cached_groesse_karte_y-y))>=0;
//		return x>=0 &&  y>=0  &&  cached_groesse_karte_x>=x  &&  cached_groesse_karte_y>=y;
	}

	inline bool ist_in_gittergrenzen(koord k) const {
	// prissi: since negative values will make the whole result negative, we can use bitwise or
	// faster, since pentiums and other long pipeline processors do not like jumps
		return (k.x|k.y|(cached_groesse_gitter_x-k.x)|(cached_groesse_gitter_y-k.y))>=0;
//		return k.x>=0 &&  k.y>=0  &&  cached_groesse_gitter_x>=k.x  &&  cached_groesse_gitter_y>=k.y;
	}

	inline bool ist_in_gittergrenzen(sint16 x, sint16 y) const {
	// prissi: since negative values will make the whole result negative, we can use bitwise or
	// faster, since pentiums and other long pipeline processors do not like jumps
		return (x|y|(cached_groesse_gitter_x-x)|(cached_groesse_gitter_y-y))>=0;
//		return x>=0 &&  y>=0  &&  cached_groesse_gitter_x>=x  &&  cached_groesse_gitter_y>=y;
	}

	inline bool ist_in_gittergrenzen(uint16 x, uint16 y) const {
		return (x<=(unsigned)cached_groesse_gitter_x && y<=(unsigned)cached_groesse_gitter_y);
	}

	/**
	* Inline because called very frequently!
	* @return Planquadrat an koordinate pos
	* @author Hj. Malthaner
	*/
	inline const planquadrat_t * lookup(const koord k) const //planquadrat = "grid square" (Babelfish)
	{
		return ist_in_kartengrenzen(k.x, k.y) ? &plan[k.x+k.y*cached_groesse_gitter_x] : 0;
		//ist in kartengrenzen = "is in map-border". (Babelfish)
	}

	/**
	 * Inline because called very frequently!
	 * @return grund an pos/hoehe
	 * @author Hj. Malthaner
	 */
	inline grund_t * lookup(const koord3d pos) const
	{
		const planquadrat_t *plan = lookup(pos.get_2d());
		return plan ? plan->get_boden_in_hoehe(pos.z) : NULL;
		//"boden in hoehe" = floor in height (Google)
	}

	/**
	 * Inline because called very frequently!
	 * @return grund at the bottom (where house will be build)
	 * @author Hj. Malthaner
	 */
	inline grund_t * lookup_kartenboden(const koord pos) const
	{
		const planquadrat_t *plan = lookup(pos);
		return plan ? plan->get_kartenboden() : NULL;
		//kartenboden = map ground (Babelfish)
	}

	/**
	 * returns the natural slope at a position
	 * uses the corner height for the best slope
	 * @author prissi
	 */
	uint8	recalc_natural_slope( const koord pos, sint8 &new_height ) const;

	// no checking, and only using the grind for calculation
	uint8	calc_natural_slope( const koord pos ) const;

	/**
	 * Wird vom Strassenbauer als Orientierungshilfe benutzt.
	 * @author Hj. Malthaner
	 */
	inline void markiere(koord3d k) { marker.markiere(lookup(k)); }
	inline void markiere(const grund_t* gr) { marker.markiere(gr); }

	/**
	 * Wird vom Strassenbauer zum Entfernen der Orientierungshilfen benutzt.
	 * @author Hj. Malthaner
	 */
	inline void unmarkiere(koord3d k) { marker.unmarkiere(lookup(k)); }
	inline void unmarkiere(const grund_t* gr) { marker.unmarkiere(gr); }

	/**
	 * Entfernt alle Markierungen.
	 * @author Hj. Malthaner
	 */
	inline void unmarkiere_alle() { marker.unmarkiere_alle(); }

	/**
	 * Testet ob der Grund markiert ist.
	 * @return Gibt true zurueck wenn der Untergrund markiert ist sonst false.
	 * @author Hj. Malthaner
	 */
	inline bool ist_markiert(koord3d k) const { return marker.ist_markiert(lookup(k)); }
	inline bool ist_markiert(const grund_t* gr) const { return marker.ist_markiert(gr); }

	// Getter/setter methods for maintaining the industry density
	inline uint32 get_target_industry_density() const { return ((uint32)finance_history_month[0][WORLD_CITICENS] * (sint64)industry_density_proportion) / 1000000ll; }
	inline uint32 get_actual_industry_density() const { return actual_industry_density; }
	
	inline void decrease_actual_industry_density(uint32 value) { actual_industry_density -= value; }
	inline void increase_actual_industry_density(uint32 value) { actual_industry_density += value; }

	 /**
	 * Initialize map.
	 * @param sets game settings
	 * @param preselected_players defines which players the user has selected before he started the game
	 * @author Hj. Malthaner
	 */
	void init(settings_t*, sint8 const* heights);

	void init_felder();

	void enlarge_map(settings_t const*, sint8 const* h_field);

	karte_t();

	~karte_t();

	// return an index to a halt (or creates a new one)
	// only used during loading
	halthandle_t get_halt_koord_index(koord k);

	/**
	 * Prueft, ob das Planquadrat an Koordinate (x,y)
	 * erniedrigt werden kann
	 * @author V. Meyer
	 */
	bool can_lower_plan_to(sint16 x, sint16 y, sint8 h) const;

	/**
	 * Prueft, ob das Planquadrat an Koordinate (x,y)
	 * erhoeht werden kann
	 * @author V. Meyer
	 */
	bool can_raise_plan_to(sint16 x, sint16 y, sint8 h) const;

	/**
	 * Prueft, ob das Planquadrat an Koordinate (x,y)
	 * geaendert werden darf. (z.B. kann Wasser nicht geaendert werden)
	 * @author Hj. Malthaner
	 */
	bool is_plan_height_changeable(sint16 x, sint16 y) const;

	/**
	 * Prueft, ob die Hoehe an Gitterkoordinate (x,y)
	 * erhoeht werden kann.
	 * @param x x-Gitterkoordinate
	 * @param y y-Gitterkoordinate
	 * @author Hj. Malthaner
	 */
	bool can_raise(sint16 x,sint16 y) const;

	/**
	 * Erhoeht die Hoehe an Gitterkoordinate (x,y) um eins.
	 * @param pos Gitterkoordinate
	 * @author Hj. Malthaner
	 */
	int raise(koord pos);

	/**
	 * Prueft, ob die Hoehe an Gitterkoordinate (x,y)
	 * erniedrigt werden kann.
	 * @param x x-Gitterkoordinate
	 * @param y y-Gitterkoordinate
	 * @author Hj. Malthaner
	 */
	bool can_lower(sint16 x,sint16 y) const;

	/**
	 * Erniedrigt die Hoehe an Gitterkoordinate (x,y) um eins.
	 * @param pos Gitterkoordinate
	 * @author Hj. Malthaner
	 */
	int lower(koord pos);

	// mostly used by AI: Ask to flatten a tile
	bool can_ebne_planquadrat(koord pos, sint8 hgt, bool keep_water=false, bool make_underwater_hill=false) const;
	bool ebne_planquadrat(spieler_t *sp, koord pos, sint8 hgt, bool keep_water=false, bool make_underwater_hill=false);

	// the convois are also handled each step => thus we keep track of them too
	void add_convoi(convoihandle_t &cnv);
	void rem_convoi(convoihandle_t& cnv);
	vector_tpl<convoihandle_t> & convoys() { return convoi_array; }

	/**
	 * Zugriff auf das Staedte Array.
	 * @author Hj. Malthaner
	 */
	const weighted_vector_tpl<stadt_t*>& get_staedte() const { return stadt; }
	stadt_t *get_town_at(const uint32 weight) { return stadt.at_weight(weight); }
	uint32 get_town_list_weight() const { return stadt.get_sum_weight(); }

	void add_stadt(stadt_t *s);
	bool rem_stadt(stadt_t *s);

	/* tourist attraction list */
	void add_ausflugsziel(gebaeude_t *gb);
	void remove_ausflugsziel(gebaeude_t *gb);
	const weighted_vector_tpl<gebaeude_t*> &get_ausflugsziele() const {return ausflugsziele; }

	void add_label(koord pos) { if (!labels.is_contained(pos)) labels.append(pos); }
	void remove_label(koord pos) { labels.remove(pos); }
	const slist_tpl<koord>& get_label_list() const { return labels; }

	bool add_fab(fabrik_t *fab);
	bool rem_fab(fabrik_t *fab);

	int get_fab_index(fabrik_t* fab)  const { return fab_list.index_of(fab); }
	const vector_tpl<fabrik_t*>& get_fab_list() const { return fab_list; }
	vector_tpl<fabrik_t*>& access_fab_list() { return fab_list; }

	// Returns a list of goods produced by factories that exist in current game
	const vector_tpl<const ware_besch_t*> &get_goods_list();

	/**
	 * sucht naechstgelegene Stadt an Position i,j
	 * @author Hj. Malthaner
	 */

	stadt_t * suche_naechste_stadt(koord pos) const;
	
	// Returns the city at the position given.
	// Returns NULL if there is no city there.
	stadt_t * get_city(koord pos) const;

	bool cannot_save() const { return nosave; }
	void set_nosave() { nosave = true; }
	void set_nosave_warning() { nosave_warning = true; }

	// rotate map view by 90 degrees
	void rotate90();

	bool sync_add(sync_steppable *obj);
	bool sync_remove(sync_steppable *obj);

	void sync_step(long delta_t, bool sync, bool display );	// advance also the timer

	void step();

	inline planquadrat_t *access(int i, int j) const {
		return ist_in_kartengrenzen(i, j) ? &plan[i + j*cached_groesse_gitter_x] : NULL;
	}

	inline planquadrat_t *access(koord k) const {
		return ist_in_kartengrenzen(k) ? &plan[k.x + k.y*cached_groesse_gitter_x] : NULL;
	}

	/**
	 * @return Hoehe am Gitterpunkt i,j
	 * "Height at the grid point" (Google)
	 * @author Hj. Malthaner
	 */
	inline sint8 lookup_hgt(koord k) const {
		return ist_in_gittergrenzen(k.x, k.y) ? grid_hgts[k.x + k.y*(cached_groesse_gitter_x+1)]*Z_TILE_STEP : grundwasser;
	}

	/**
	 * Sets grid height.
	 * Never set grid_hgts manually, always use this method!
	 * @author Hj. Malthaner
	 */
	void set_grid_hgt(koord k, sint16 hgt) { grid_hgts[k.x + k.y*(uint32)(cached_groesse_gitter_x+1)] = (hgt/Z_TILE_STEP); }

	/**
	 * @return Minimale Hoehe des Planquadrates i,j
	 * @author Hj. Malthaner
	 */
	sint8 min_hgt(koord pos) const;

	/**
	 * @return Maximale Hoehe des Planquadrates i,j
	 * @author Hj. Malthaner
	 */
	sint8 max_hgt(koord pos) const;

	/**
	 * @return true, wenn Platz an Stelle pos mit Groesse dim Wasser ist
	 * @author V. Meyer
	 */
	bool ist_wasser(koord pos, koord dim) const;

	/**
	 * @return true, wenn Platz an Stelle i,j mit Groesse w,h bebaubar
	 * @author Hj. Malthaner
	 */
	bool ist_platz_frei(koord pos, sint16 w, sint16 h, int *last_y, climate_bits cl) const;

	/**
	 * @return eine Liste aller bebaubaren Plaetze mit Groesse w,h
	 * only used for town creation at the moment
	 * @author Hj. Malthaner
	 */
	slist_tpl<koord> * finde_plaetze(sint16 w, sint16 h, climate_bits cl, sint16 old_x, sint16 old_y) const;

	/**
	 * Spielt den Sound, wenn die Position im sichtbaren Bereich liegt.
	 * Spielt weiter entfernte Sounds leiser ab.
	 * @param pos Position an der das Ereignis stattfand
	 * @param idx Index of the sound
	 * @author Hj. Malthaner
	 */
	bool play_sound_area_clipped(koord pos, uint16 idx) const;

	void mute_sound( bool state ) { is_sound = !state; }

	/**
	 * Saves the map to a file
	 * @param filename name of the file to write
	 * @author Hj. Malthaner
	 */
	void speichern(const char *filename, const char *version, const char *ex_version, bool silent);

	/**
	 * Loads a map from a file
	 * @param filename name of the file to read
	 * @author Hj. Malthaner
	 */
	bool laden(const char *filename);

	/**
	 * Creates a map from a heightfield
	 * @param sets game settings
	 * @author Hj. Malthaner
	 */
	void load_heightfield(settings_t*);

	void beenden(bool b);

	/**
	 * main loop with event handling;
	 * returns false to exit
	 * @author Hj. Malthaner
	 */

	uint16 get_base_pathing_counter() const { return base_pathing_counter; }

	bool interactive(uint32 quit_month);

	uint32 get_sync_steps() const { return sync_steps; }

	// check whether checklist is available, ie given sync_step is not too far into past
	bool is_checklist_available(const uint32 sync_step) const { return sync_step + LAST_CHECKLISTS_COUNT > sync_steps; }
	const checklist_t& get_checklist_at(const uint32 sync_step) const { return LCHKLST(sync_step); }
	void set_checklist_at(const uint32 sync_step, const checklist_t &chklst) { LCHKLST(sync_step) = chklst; }

	const checklist_t& get_last_checklist() const { return LCHKLST(sync_steps); }
	uint32 get_last_checklist_sync_step() const { return sync_steps; }

	void command_queue_append(network_world_command_t*) const;

	void clear_command_queue() const;

	void network_disconnect();

	sint32 get_citycar_speed_average() const { return citycar_speed_average; }

	void set_recheck_road_connexions() { recheck_road_connexions = true; }

	/**
	 * These methods return an estimated
	 * road speed based on the average 
	 * speed of road traffic and the 
	 * speed limit of the appropriate type
	 * of road.
	 */
	uint16 get_generic_road_speed_city() const { return generic_road_speed_city; }
	uint16 get_generic_road_speed_intercity() const { return generic_road_speed_intercity; };

	sint32 calc_generic_road_speed(const weg_besch_t* besch);

	uint32 get_max_road_check_depth() const { return max_road_check_depth; }

	/**
	 * to identify the current map
	 */
	uint32 get_map_counter() const { return map_counter; }

	void set_map_counter(uint32 new_map_counter);

	/**
	 * called by the server before sending the sync commands
	 */

	uint32 generate_new_map_counter() const;

	uint8 step_next_private_car_update_month() 
	{ 
		uint8 tmp = next_private_car_update_month;
		next_private_car_update_month ++ ; 
		if(next_private_car_update_month > 12)
		{
			next_private_car_update_month = 1;
		}
		return tmp;
	}
	
	void sprintf_ticks(char *p, size_t size, sint64 ticks) const;
	void sprintf_time(char *p, size_t size, uint32 seconds) const;
	
	// @author: jamespetts
	void set_scale();


#ifdef DEBUG_SIMRAND_CALLS
	static vector_tpl<const char*> random_callers;
#endif

private:

	void calc_generic_road_speed_city() { generic_road_speed_city = calc_generic_road_speed(city_road); }
	void calc_generic_road_speed_intercity();
	void calc_max_road_check_depth();
};

#endif

