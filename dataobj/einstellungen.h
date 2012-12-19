#ifndef dataobj_einstellungen_h
#define dataobj_einstellungen_h

#include <string>
#include "../simtypes.h"
#include "../simconst.h"
#include "../simunits.h"
#include "livery_scheme.h"

/**
 * Spieleinstellungen
 * "Game options"
 *
 * Hj. Malthaner
 *
 * April 2000
 */

class spieler_t;
class loadsave_t;
class tabfile_t;
class weg_besch_t;

// these are the only classes, that are allowed to modfy elements from settings_t
// for all remaing special cases there are the set_...() routines
class settings_general_stats_t;
class settings_routing_stats_t;
class settings_economy_stats_t;
class settings_costs_stats_t;
class settings_climates_stats_t;
class climate_gui_t;
class welt_gui_t;


struct road_timeline_t
{
	char name[64];
	uint16 intro;
	uint16 retire;
};

template <class T>
class vector_with_ptr_ownership_tpl : public vector_tpl<T*> 
{
public:
	vector_with_ptr_ownership_tpl(uint32 size = 0) : 
		vector_tpl<T*>(size) {}

	vector_with_ptr_ownership_tpl( vector_with_ptr_ownership_tpl const& src ) :
		vector_tpl<T*>( src.get_count() ) {
		ITERATE( src, i ) {
			append( new T( *src[i] ) );
		}
	}

	vector_with_ptr_ownership_tpl& operator=( vector_with_ptr_ownership_tpl const& other ) { 
		vector_with_ptr_ownership_tpl tmp(other); 
		swap( static_cast<vector_tpl<T*>&>(tmp), static_cast<vector_tpl<T*>&>(*this) ); 
		return *this;
	}

	void clear() {
		clear_ptr_vector(*this);
	}

	~vector_with_ptr_ownership_tpl() {
		clear();
	}
};

class settings_t
{
friend class settings_general_stats_t;
friend class settings_routing_stats_t;
friend class settings_economy_stats_t;
friend class settings_costs_stats_t;
friend class settings_climates_stats_t;
friend class climate_gui_t;
friend class welt_gui_t;

private:
	sint32 groesse_x, groesse_y;
	sint32 nummer;

	/* new setting since version 0.85.01
	 * @author prissi
	 * not used any more:    sint32 industrie_dichte;
	 */
	sint32 land_industry_chains;
	sint32 electric_promille;
	sint32 tourist_attractions;

	sint32 anzahl_staedte;
	sint32 mittlere_einwohnerzahl;

	// town growth factors
	sint32 passenger_multiplier;
	sint32 mail_multiplier;
	sint32 goods_multiplier;
	sint32 electricity_multiplier;

	// Also there are size dependen factors (0=no growth)
	sint32 growthfactor_small;
	sint32 growthfactor_medium;
	sint32 growthfactor_large;

	uint32 industry_increase;
	uint32 city_isolation_factor;

	// percentage of routing
	sint16 factory_worker_percentage;
	sint16 tourist_percentage;

	// radius for city trips of different distance ranges
	sint16 factory_worker_radius;
	sint32 factory_worker_minimum_towns;
	sint32 factory_worker_maximum_towns;

	// Knightly : number of periods for averaging the amount of arrived pax/mail at factories
	uint16 factory_arrival_periods;

	// Knightly : whether factory pax/mail demands are enforced
	bool factory_enforce_demand;

	uint16 station_coverage_size;

	/**
	 * ab welchem level erzeugen gebaeude verkehr ?
	 */
	sint32 verkehr_level;

	/**
	 * sollen Fussgaenger angezeigt werden ?
	 */
	sint32 show_pax;

	 /**
	 * waterlevel, climate borders, lowest snow in winter
	 */

	sint16 grundwasser;
	sint16 climate_borders[MAX_CLIMATES];
	sint16 winter_snowline;

	double max_mountain_height;                  //01-Dec-01        Markus Weber    Added
	double map_roughness;                        //01-Dec-01        Markus Weber    Added

	// river stuff
	sint16 river_number;
	sint16 min_river_length;
	sint16 max_river_length;

	// forest stuff
	uint8 forest_base_size;
	uint8 forest_map_size_divisor;
	uint8 forest_count_divisor;
	uint16 forest_inverse_spare_tree_density;
	uint8 max_no_of_trees_on_square;
	uint16 tree_climates;
	uint16 no_tree_climates;
	bool no_trees;

	// game mechanics
	uint8 allow_player_change;
	uint8 use_timeline;
	sint16 starting_year;
	sint16 starting_month;
	sint16 bits_per_month;

	std::string filename;

	bool beginner_mode;
	sint32 beginner_price_factor;

	bool just_in_time;

	// default 0, will be incremented after each 90 degree rotation until 4
	uint8 rotation;

	sint16 origin_x, origin_y;

	sint32 passenger_factor;

	sint16 factory_spacing;

	/* prissi: crossconnect all factories (like OTTD and similar games) */
	bool crossconnect_factories;

	/* prissi: crossconnect all factories (like OTTD and similar games) */
	sint16 crossconnect_factor;

	/**
	* Zuf�llig Fussg�nger in den St�dten erzeugen?
	*
	* @author Hj. Malthaner
	*/
	bool fussgaenger;

	sint32 stadtauto_duration;

	bool freeplay;

	sint64 starting_money;

	struct yearmoney
	{
		sint16 year;
		sint64 money;
		bool interpol;
	};

	yearmoney startingmoneyperyear[10];

	uint16 num_city_roads;
	road_timeline_t city_roads[10];
	uint16 num_intercity_roads;
	road_timeline_t intercity_roads[10];

	/**
	 * Use numbering for stations?
	 *
	 * @author Hj. Malthaner
	 */
	bool numbered_stations;

	/* prissi: maximum number of steps for breath search */
	sint32 max_route_steps;

	// max steps for good routing
	sint32 max_hops;

	/* prissi: maximum number of steps for breath search */
	sint32 max_transfers;

	/**
	 * multiplier for steps on diagonal:
	 * 1024: TT-like, faktor 2, vehicle will be too long and too fast
	 * 724: correct one, faktor sqrt(2)
	 */
	uint16 pak_diagonal_multiplier;

	// names of the stations ...
	char language_code_names[4];

	// true, if the different caacities (passengers/mail/freight) are counted seperately
	bool seperate_halt_capacities;

	vector_with_ptr_ownership_tpl<livery_scheme_t> livery_schemes;

	// Whether passengers might walk between stops en route.
	// @author: jamespetts, August 2011
	bool allow_routing_on_foot;

	/**
	 * If true, players will not be charged
	 * for using roads owned by the public
	 * service player. Other way types are
	 * not affected.
	 * @author: jamespetts, October 2011
	 */
	bool toll_free_public_roads;

	uint8 max_elevated_way_building_level;

	/**
	 * If players allow access to their roads
	 * by private cars, they will pay this
	 * much of a toll for every tile
	 */
	sint64 private_car_toll_per_tile;

	/**
	 * If this is enabled, player roads
	 * built within city limits will on
	 * building be classed as unowned so
	 * that other players may use/modify
	 * them. This will not affect other 
	 * ways built outside city limits
	 * but which subsequently fall within
	 * them.
	 */
	bool towns_adopt_player_roads;

public:
	//Cornering settings
	//@author: jamespetts
	
	//The array index corresponds
	//to the waytype index.
	
	uint32 max_corner_limit[10];
	uint32 min_corner_limit[10];
	uint16 max_corner_adjustment_factor[10];
	uint16 min_corner_adjustment_factor[10];
	uint8 min_direction_steps[10];
	uint8 max_direction_steps[10];
	uint8 curve_friction_factor[10];

	/* if set, goods will avoid being routed over overcrowded stops */
	bool avoid_overcrowding;

	// The longest time that a passenger is
	// prepared to wait for transport.
	// @author: jamespetts
	uint32 passenger_max_wait;

	uint8 max_rerouting_interval_months;

	//@author: jamespetts
	// Revenue calibration settings
	uint16 min_bonus_max_distance;
	uint16 max_bonus_min_distance;
	uint16 median_bonus_distance;
	uint16 max_bonus_multiplier_percent;
	uint16 meters_per_tile;
	// We need it often(every vehikel_basis_t::fahre_basis call), so we cache it.
	uint32 steps_per_km;

	uint8 tolerable_comfort_short;
	uint8 tolerable_comfort_median_short;
	uint8 tolerable_comfort_median_median;
	uint8 tolerable_comfort_median_long;
	uint8 tolerable_comfort_long;
	uint16 tolerable_comfort_short_minutes;
	uint16 tolerable_comfort_median_short_minutes;
	uint16 tolerable_comfort_median_median_minutes;
	uint16 tolerable_comfort_median_long_minutes;
	uint16 tolerable_comfort_long_minutes;
	uint8 max_luxury_bonus_differential;
	uint8 max_discomfort_penalty_differential;
	uint16 max_luxury_bonus_percent;
	uint16 max_discomfort_penalty_percent;

	uint16 catering_min_minutes;
	uint16 catering_level1_minutes;
	uint16 catering_level1_max_revenue;
	uint16 catering_level2_minutes;
	uint16 catering_level2_max_revenue;
	uint16 catering_level3_minutes;
	uint16 catering_level3_max_revenue;
	uint16 catering_level4_minutes;
	uint16 catering_level4_max_revenue;
	uint16 catering_level5_minutes;
	uint16 catering_level5_max_revenue;

	uint16 tpo_min_minutes;
	uint16 tpo_revenue;

	//@author: jamespetts
	// Obsolete vehicle maintenance cost increases
	uint16 obsolete_running_cost_increase_percent;
	uint16 obsolete_running_cost_increase_phase_years;

	//@author: jamespetts
	// Passenger destination ranges
	// Use to set the extent to which passengers prefer local, medium, or long-range destinations.
	// The distances can (and probably should) overlap.
	uint32 local_passengers_min_distance;
	uint32 local_passengers_max_distance;
	uint32 midrange_passengers_min_distance;
	uint32 midrange_passengers_max_distance;
	uint32 longdistance_passengers_min_distance;
	uint32 longdistance_passengers_max_distance;
	
	// @author: jamespetts
	// Private car settings
	uint8 always_prefer_car_percent;
	uint8 base_car_preference_percent;
	uint8 congestion_density_factor;

	//@author: jamespetts
	// Passenger routing settings
	uint8 passenger_routing_packet_size;
	uint8 max_alternative_destinations;
	uint8 passenger_routing_local_chance;
	uint8 passenger_routing_midrange_chance;

	//@author: jamespetts
	// Factory retirement settings
	uint16 factory_max_years_obsolete;

	//@author: jamespetts
	// Insolvency and debt settings
	uint8 interest_rate_percent;
	bool allow_bankruptsy;
	bool allow_purhcases_when_insolvent;

	// Reversing settings
	//@author: jamespetts
	uint16 unit_reverse_time;
	uint16 hauled_reverse_time;
	uint16 turntable_reverse_time;

	//@author: jamespetts
	uint16 global_power_factor_percent; 
	
	// Whether and how weight limits are enforced
	// @author: jamespetts
	uint8 enforce_weight_limits;

	// Adjustment of the speed bonus for use with
	// speedbonus.tab files from Simutrans-Standard
	// @author: jamespetts
	uint16 speed_bonus_multiplier_percent;

	bool allow_airports_without_control_towers;

	/**
	 * The shortest time that passengers/goods can
	 * wait at an airport before boarding an aircraft.
	 * Waiting times are higher at airports because of
	 * the need to check-in, undergo security checks,
	 * etc.
	 * @author: jamespetts, August 2011
	 */
	uint16 min_wait_airport;
	
private:

	// true, if this pak should be used with extensions (default)
	bool with_private_paks;
	
public:

	// The ranges for the journey time tolerance for passengers.
	// @author: jamespetts
	uint16 min_local_tolerance;
	uint16 max_local_tolerance;
	uint16 min_midrange_tolerance;
	uint16 max_midrange_tolerance;
	uint16 min_longdistance_tolerance;
	uint16 max_longdistance_tolerance;

	// The walking distance in tiles
	// that people are prepared to 
	// tolerate.
	// @author: jamespetts, December 2009
	uint16 max_walking_distance;
	
private:

	// if true, you can buy obsolete stuff
	bool allow_buying_obsolete_vehicles;
	// vehicle value is decrease by this factor/1000 when a vehicle leaved the depot
	sint16 used_vehicle_reduction;

	uint32 random_counter;
	uint32 frames_per_second;	// only used in network mode ...
	uint32 frames_per_step;
	uint32 server_frames_ahead;

	bool drive_on_left;
	bool signals_on_left;

	// The fraction of running costs (etc.) charged for going on other players' ways
	// and using their air and sea ports.
	sint32 way_toll_runningcost_percentage;
	sint32 way_toll_waycost_percentage;
	sint32 way_toll_revenue_percentage;
	sint32 seaport_toll_revenue_percentage;
	sint32 airport_toll_revenue_percentage;

	// Whether non-public players are allowed to make stops and ways public.
	bool allow_making_public;
#ifndef NETTOOL
	float32e8_t simtime_factor;
	float32e8_t meters_per_step;
	float32e8_t steps_per_meter;
	float32e8_t seconds_per_tick;
#endif
public:
	/* the big cost section */
	sint32 maint_building;	// normal building

	sint64 cst_multiply_dock;
	sint64 cst_multiply_station;
	sint64 cst_multiply_roadstop;
	sint64 cst_multiply_airterminal;
	sint64 cst_multiply_post;
	sint64 cst_multiply_headquarter;
	sint64 cst_depot_rail;
	sint64 cst_depot_road;
	sint64 cst_depot_ship;
	sint64 cst_depot_air;

	// alter landscape
	sint64 cst_buy_land;
	sint64 cst_alter_land;
	sint64 cst_set_slope;
	sint64 cst_found_city;
	sint64 cst_multiply_found_industry;
	sint64 cst_remove_tree;
	sint64 cst_multiply_remove_haus;
	sint64 cst_multiply_remove_field;
	sint64 cst_transformer;
	sint64 cst_maintain_transformer;

	// costs for the way searcher
	sint32 way_count_straight;
	sint32 way_count_curve;
	sint32 way_count_double_curve;
	sint32 way_count_90_curve;
	sint32 way_count_slope;
	sint32 way_count_tunnel;
	uint32 way_max_bridge_len;
	sint32 way_count_leaving_road;

	// true if active
	bool automaten[MAX_PLAYER_COUNT];
	// 0 = emtpy, otherwise some vaule from simplay
	uint8 spieler_type[MAX_PLAYER_COUNT];

	// If true, the old (faster) method of
	// city growth is used. If false (default),
	// the new, more accurate, method of city
	// growth is used.
	bool quick_city_growth;

	// The new (8.0) system for private cars checking
	// whether their destination is reachable can have
	// an adverse effect on performance. Allow it to 
	// be disabled. 
	bool assume_everywhere_connected_by_road;

	uint16 default_increase_maintenance_after_years[17];

	uint32 city_threshold_size;
	uint32 capital_threshold_size;
	uint32 max_small_city_size;
	uint32 max_city_size;

	// player color suggestions for new games
	bool default_player_color_random;
	uint8 default_player_color[MAX_PLAYER_COUNT][2];

	enum spacing_shift_mode_t { SPACING_SHIFT_DISABLED = 0, SPACING_SHIFT_PER_LINE, SPACING_SHIFT_PER_STOP};
	uint8 spacing_shift_mode;
	sint16 spacing_shift_divisor;

	/**
	 * If map is read from a heightfield, this is the name of the heightfield.
	 * Set to empty string in order to avoid loading.
	 * @author Hj. Malthaner
	 */
	std::string heightfield;

	settings_t();

	void rdwr(loadsave_t *file);

	void copy_city_road(settings_t const& other);

	// init form this file ...
	void parse_simuconf( tabfile_t &simuconf, sint16 &disp_width, sint16 &disp_height, sint16 &fullscreen, std::string &objfilename );

	void set_groesse_x(sint32 g) {groesse_x=g;}
	void set_groesse_y(sint32 g) {groesse_y=g;}
	void set_groesse(sint32 x, sint32 y) {groesse_x = x; groesse_y=y;}
	sint32 get_groesse_x() const {return groesse_x;}
	sint32 get_groesse_y() const {return groesse_y;}

	sint32 get_karte_nummer() const {return nummer;}

	void set_land_industry_chains(sint32 d) {land_industry_chains=d;}
	sint32 get_land_industry_chains() const {return land_industry_chains;}

	sint32 get_electric_promille() const {return electric_promille;}

	void set_tourist_attractions( sint32 n ) { tourist_attractions = n; }
	sint32 get_tourist_attractions() const {return tourist_attractions;}

	void set_anzahl_staedte(sint32 n) {anzahl_staedte=n;}
	sint32 get_anzahl_staedte() const {return anzahl_staedte;}

	void set_mittlere_einwohnerzahl( sint32 n ) {mittlere_einwohnerzahl = n;}
	sint32 get_mittlere_einwohnerzahl() const {return mittlere_einwohnerzahl;}

	void set_verkehr_level(sint32 l) {verkehr_level=l;}
	sint32 get_verkehr_level() const {return verkehr_level;}

	void set_show_pax(bool yesno) {show_pax=yesno;}
	bool get_show_pax() const {return show_pax != 0;}

	sint16 get_grundwasser() const {return grundwasser;}

	double get_max_mountain_height() const {return max_mountain_height;}

	double get_map_roughness() const {return map_roughness;}

	uint16 get_station_coverage() const {return station_coverage_size;}

	void set_allow_player_change(char n) {allow_player_change=n;}	// prissi, Oct-2005
	uint8 get_allow_player_change() const {return allow_player_change;}

	void set_use_timeline(char n) {use_timeline=n;}	// prissi, Oct-2005
	uint8 get_use_timeline() const {return use_timeline;}

	void set_starting_year( sint16 n ) { starting_year = n; }
	sint16 get_starting_year() const {return starting_year;}

	sint16 get_starting_month() const {return starting_month;}

	sint16 get_bits_per_month() const {return bits_per_month;}

	void set_filename(const char *n) {filename=n;}	// prissi, Jun-06
	const char* get_filename() const { return filename.c_str(); }

	bool get_beginner_mode() const {return beginner_mode;}

	void set_just_in_time(bool b) { just_in_time = b; }
	bool get_just_in_time() const {return just_in_time;}

	void set_default_climates();
	const sint16 *get_climate_borders() const { return climate_borders; }

	sint16 get_winter_snowline() const {return winter_snowline;}

	void rotate90() {
		rotation = (rotation+1)&3;
		set_groesse( groesse_y, groesse_x );
	}
	uint8 get_rotation() const { return rotation; }

	void set_origin_x(sint16 x) { origin_x = x; }
	void set_origin_y(sint16 y) { origin_y = y; }
	sint16 get_origin_x() const { return origin_x; }
	sint16 get_origin_y() const { return origin_y; }

	bool is_freeplay() const { return freeplay; }
	void set_freeplay( bool f ) { freeplay = f; }

	sint32 get_max_route_steps() const { return max_route_steps; }
	sint32 get_max_hops() const { return max_hops; }
	sint32 get_max_transfers() const { return max_transfers; }

	sint64 get_starting_money(sint16 year) const;

	bool get_random_pedestrians() const { return fussgaenger; }
	void set_random_pedestrians( bool f ) { fussgaenger = f; }

	sint16 get_factory_spacing() const { return factory_spacing; }
	sint16 get_crossconnect_factor() const { return crossconnect_factor; }
	bool is_crossconnect_factories() const { return crossconnect_factories; }

	bool get_numbered_stations() const { return numbered_stations; }

	sint32 get_stadtauto_duration() const { return stadtauto_duration; }

	sint32 get_beginner_price_factor() const { return beginner_price_factor; }

	const weg_besch_t *get_city_road_type( uint16 year );
	const weg_besch_t *get_intercity_road_type( uint16 year );

	void set_pak_diagonal_multiplier(uint16 n) { pak_diagonal_multiplier = n; }
	uint16 get_pak_diagonal_multiplier() const { return pak_diagonal_multiplier; }

	int get_name_language_id() const;
	void set_name_language_iso( const char *iso ) {
		language_code_names[0] = iso[0];
		language_code_names[1] = iso[1];
		language_code_names[2] = 0;
	}

	void set_player_active(uint8 i, bool b) { automaten[i] = b; }
	void set_player_type(uint8 i, uint8 t) { spieler_type[i] = t; }
	uint8 get_player_type(uint8 i) const { return spieler_type[i]; }

	bool is_seperate_halt_capacities() const { return seperate_halt_capacities ; }

	uint16 get_min_bonus_max_distance() const { return min_bonus_max_distance; }
	void   set_min_bonus_max_distance(uint16 value) { min_bonus_max_distance = value; }
	uint16 get_median_bonus_distance() const { return median_bonus_distance; }
	void   set_median_bonus_distance(uint16 value) { median_bonus_distance = value; }
	uint16 get_max_bonus_min_distance() const { return max_bonus_min_distance; }
	void   set_max_bonus_min_distance(uint16 value) { max_bonus_min_distance = value; }

	uint16 get_max_bonus_multiplier_percent() const { return max_bonus_multiplier_percent; }
	void   set_max_bonus_multiplier_percent(uint16 value) { max_bonus_multiplier_percent = value; }

	uint16 get_meters_per_tile() const { return meters_per_tile; }
	void   set_meters_per_tile(uint16 value);
	uint32 get_steps_per_km() const { return steps_per_km; }
//	void   set_distance_per_tile_percent(uint16 value) { meters_per_tile = value * 10; }

	uint8  get_tolerable_comfort_short() const { return tolerable_comfort_short; }
	void   set_tolerable_comfort_short(uint8 value) { tolerable_comfort_short = value; }
	uint16 get_tolerable_comfort_short_minutes() const { return tolerable_comfort_short_minutes; }
	void   set_tolerable_comfort_short_minutes(uint16 value) { tolerable_comfort_short_minutes = value; }

	uint8  get_tolerable_comfort_median_short() const { return tolerable_comfort_median_short; }
	void   set_tolerable_comfort_median_short(uint8 value) { tolerable_comfort_median_short = value; }
	uint16 get_tolerable_comfort_median_short_minutes() const { return tolerable_comfort_median_short_minutes; }
	void   set_tolerable_comfort_median_short_minutes(uint16 value) { tolerable_comfort_median_short_minutes = value; }

	uint8  get_tolerable_comfort_median_median() const { return tolerable_comfort_median_median; }
	void   set_tolerable_comfort_median_median(uint8 value) { tolerable_comfort_median_median = value; }
	uint16 get_tolerable_comfort_median_median_minutes() const { return tolerable_comfort_median_median_minutes; }
	void   set_tolerable_comfort_median_median_minutes(uint16 value) { tolerable_comfort_median_median_minutes = value; }

	uint8  get_tolerable_comfort_median_long() const { return tolerable_comfort_median_long; }
	void   set_tolerable_comfort_median_long(uint8 value) { tolerable_comfort_median_long = value; }
	uint16 get_tolerable_comfort_median_long_minutes() const { return tolerable_comfort_median_long_minutes; }
	void   set_tolerable_comfort_median_long_minutes(uint16 value) { tolerable_comfort_median_long_minutes = value; }

	uint8  get_tolerable_comfort_long() const { return tolerable_comfort_long; }
	void   set_tolerable_comfort_long(uint8 value) { tolerable_comfort_long = value; }
	uint16 get_tolerable_comfort_long_minutes() const { return tolerable_comfort_long_minutes; }
	void   set_tolerable_comfort_long_minutes(uint16 value) { tolerable_comfort_long_minutes = value; }

	uint16 get_max_luxury_bonus_percent() const { return max_luxury_bonus_percent; }
	void   set_max_luxury_bonus_percent(uint16 value) { max_luxury_bonus_percent = value; }
	uint8  get_max_luxury_bonus_differential() const { return max_luxury_bonus_differential; }
	void   set_max_luxury_bonus_differential(uint8 value) { max_luxury_bonus_differential = value; }

	uint16 get_max_discomfort_penalty_percent() const { return max_discomfort_penalty_percent; }
	void   set_max_discomfort_penalty_percent(uint16 value) { max_discomfort_penalty_percent = value; }
	uint8  get_max_discomfort_penalty_differential() const { return max_discomfort_penalty_differential; }
	void   set_max_discomfort_penalty_differential(uint8 value) { max_discomfort_penalty_differential = value; }

	uint16 get_catering_min_minutes() const { return catering_min_minutes; }
	void   set_catering_min_minutes(uint16 value) { catering_min_minutes = value; }

	uint16 get_catering_level1_minutes() const { return catering_level1_minutes; }
	void   set_catering_level1_minutes(uint16 value) { catering_level1_minutes = value; }
	uint16 get_catering_level1_max_revenue() const { return catering_level1_max_revenue; }
	void   set_catering_level1_max_revenue(uint16 value) { catering_level1_max_revenue = value; }

	uint16 get_catering_level2_minutes() const { return catering_level2_minutes; }
	void   set_catering_level2_minutes(uint16 value) { catering_level2_minutes = value; }
	uint16 get_catering_level2_max_revenue() const { return catering_level2_max_revenue; }
	void   set_catering_level2_max_revenue(uint16 value) { catering_level2_max_revenue = value; }

	uint16 get_catering_level3_minutes() const { return catering_level3_minutes; }
	void   set_catering_level3_minutes(uint16 value) { catering_level3_minutes = value; }
	uint16 get_catering_level3_max_revenue() const { return catering_level3_max_revenue; }
	void   set_catering_level3_max_revenue(uint16 value) { catering_level3_max_revenue = value; }

	uint16 get_catering_level4_minutes() const { return catering_level4_minutes; }
	void   set_catering_level4_minutes(uint16 value) { catering_level4_minutes = value; }
	uint16 get_catering_level4_max_revenue() const { return catering_level4_max_revenue; }
	void   set_catering_level4_max_revenue(uint16 value) { catering_level4_max_revenue = value; }

	uint16 get_catering_level5_minutes() const { return catering_level5_minutes; }
	void   set_catering_level5_minutes(uint16 value) { catering_level5_minutes = value; }
	uint16 get_catering_level5_max_revenue() const { return catering_level5_max_revenue; }
	void   set_catering_level5_max_revenue(uint16 value) { catering_level5_max_revenue = value; }
	
	uint16 get_tpo_min_minutes() const { return tpo_min_minutes; }
	void   set_tpo_min_minutes(uint16 value) { tpo_min_minutes = value; }
	uint16 get_tpo_revenue() const { return tpo_revenue; }
	void   set_tpo_revenue(uint16 value) { tpo_revenue = value; }

	uint16 get_obsolete_running_cost_increase_percent() const { return obsolete_running_cost_increase_percent; }
	void   set_obsolete_running_cost_increase_percent(uint16 value) { obsolete_running_cost_increase_percent = value; }
	uint16 get_obsolete_running_cost_increase_phase_years() const { return obsolete_running_cost_increase_phase_years; }
	void   set_obsolete_running_cost_increase_phase_years(uint16 value) { obsolete_running_cost_increase_phase_years = value; }

	uint32 get_local_passengers_min_distance() const { return local_passengers_min_distance; }
	uint32 get_local_passengers_max_distance() const { return local_passengers_max_distance; }
	void   set_local_passengers_max_distance(uint16 value) { local_passengers_max_distance = value; }
	uint32 get_midrange_passengers_min_distance() const { return midrange_passengers_min_distance; }
	void   set_midrange_passengers_min_distance(uint16 value) { midrange_passengers_min_distance = value; }
	uint32 get_midrange_passengers_max_distance() const { return midrange_passengers_max_distance; }
	void   set_midrange_passengers_max_distance(uint16 value) { midrange_passengers_max_distance = value; }
	uint32 get_longdistance_passengers_min_distance() const { return longdistance_passengers_min_distance; }
	void   set_longdistance_passengers_min_distance(uint16 value) { longdistance_passengers_min_distance = value; }
	uint32 get_longdistance_passengers_max_distance() const { return longdistance_passengers_max_distance; }

	uint8 get_passenger_routing_packet_size() const { return passenger_routing_packet_size; }
	void  set_passenger_routing_packet_size(uint8 value) { passenger_routing_packet_size = value; }
	uint8 get_max_alternative_destinations() const { return max_alternative_destinations; }
	void  set_max_alternative_destinations(uint8 value) { max_alternative_destinations = value; }
	uint8 get_passenger_routing_local_chance() const { return passenger_routing_local_chance; }
	void  set_passenger_routing_local_chance(uint8 value) { passenger_routing_local_chance = value; }
	uint8 get_passenger_routing_midrange_chance() const { return passenger_routing_midrange_chance; }
	void  set_passenger_routing_midrange_chance(uint8 value) { passenger_routing_midrange_chance = value; }

	uint8 get_always_prefer_car_percent() const { return always_prefer_car_percent; }
	uint8 get_base_car_preference_percent () const { return base_car_preference_percent; }
	uint8 get_congestion_density_factor () const { return congestion_density_factor; }

	sint32 get_max_corner_limit(waytype_t waytype) const { return kmh_to_speed(max_corner_limit[waytype]); }
	sint32 get_min_corner_limit (waytype_t waytype) const { return kmh_to_speed(min_corner_limit[waytype]); }
	uint16 get_max_corner_adjustment_factor (waytype_t waytype) const { return max_corner_adjustment_factor[waytype]; }
	uint16 get_min_corner_adjustment_factor (waytype_t waytype) const {  return  min_corner_adjustment_factor[waytype]; }
	uint8 get_min_direction_steps (waytype_t waytype) const { return min_direction_steps[waytype]; }
	uint8 get_max_direction_steps (waytype_t waytype) const { return max_direction_steps[waytype]; }
	uint8 get_curve_friction_factor (waytype_t waytype) const { return curve_friction_factor[waytype]; }

	uint16 get_factory_max_years_obsolete() const { return factory_max_years_obsolete; }

	uint8 get_interest_rate_percent() const { return interest_rate_percent; }
	bool bankruptsy_allowed() const { return allow_bankruptsy; }
	bool insolvent_purchases_allowed() const { return allow_purhcases_when_insolvent; }

	uint16 get_unit_reverse_time() const { return unit_reverse_time; }
	uint16 get_hauled_reverse_time() const { return hauled_reverse_time; }
	uint16 get_turntable_reverse_time() const { return turntable_reverse_time; }

	uint16 get_global_power_factor_percent() const { return global_power_factor_percent; }
	void set_global_power_factor_percent(uint16 value) { global_power_factor_percent = value; }

	uint8 get_enforce_weight_limits() const { return enforce_weight_limits; }

	uint16 get_speed_bonus_multiplier_percent() const { return speed_bonus_multiplier_percent; }

	bool get_allow_airports_without_control_towers() const { return allow_airports_without_control_towers; }
	void set_allow_airports_without_control_towers(bool value) { allow_airports_without_control_towers = value; }

	// allowed modes are 0,1,2
	enum { TO_PREVIOUS=0, TO_TRANSFER, TO_DESTINATION };


	bool is_avoid_overcrowding() const { return avoid_overcrowding; }

	uint32 get_passenger_max_wait() const { return passenger_max_wait; }

	uint8 get_max_rerouting_interval_months() const { return max_rerouting_interval_months; }

	sint16 get_river_number() const { return river_number; }
	sint16 get_min_river_length() const { return min_river_length; }
	sint16 get_max_river_length() const { return max_river_length; }

	// true, if this pak should be used with extensions (default)
	void set_with_private_paks(bool b ) {with_private_paks = b;}
	bool get_with_private_paks() const { return with_private_paks; }

	// @author: jamespetts
	uint16 get_min_local_tolerance() const { return min_local_tolerance; }
	void set_min_local_tolerance(uint16 value) { min_local_tolerance = value; }
	uint16 get_max_local_tolerance() const { return max_local_tolerance; }
	void set_max_local_tolerance(uint16 value) { max_local_tolerance = value; }
	uint16 get_min_midrange_tolerance() const { return min_midrange_tolerance; }
	void set_min_midrange_tolerance(uint16 value) { min_midrange_tolerance = value; }
	uint16 get_max_midrange_tolerance() const { return max_midrange_tolerance; }
	void set_max_midrange_tolerance(uint16 value) { max_midrange_tolerance = value; }
	uint16 get_min_longdistance_tolerance() const { return min_longdistance_tolerance; }
	void set_min_longdistance_tolerance(uint16 value){ min_longdistance_tolerance = value; }
	uint16 get_max_longdistance_tolerance() const { return max_longdistance_tolerance; }
	void set_max_longdistance_tolerance(uint16 value) { max_longdistance_tolerance = value; }

	sint32 get_passenger_factor() const { return passenger_factor; }

	// town growth stuff
	sint32 get_passenger_multiplier() const { return passenger_multiplier; }
	sint32 get_mail_multiplier() const { return mail_multiplier; }
	sint32 get_goods_multiplier() const { return goods_multiplier; }
	sint32 get_electricity_multiplier() const { return electricity_multiplier; }

	// Also there are size dependen factors (0=no growth)
	sint32 get_growthfactor_small() const { return growthfactor_small; }
	sint32 get_growthfactor_medium() const { return growthfactor_medium; }
	sint32 get_growthfactor_large() const { return growthfactor_large; }

	// percentage of passengers for different kinds of trips
	sint16 get_factory_worker_percentage() const { return factory_worker_percentage; }
	sint16 get_tourist_percentage() const { return tourist_percentage; }

	// radius from factories to get workers from towns (usually set to 77 but 1/8 of map size may be meaningful too)
	uint16 get_factory_worker_radius() const { return factory_worker_radius; }

	// any factory will be connected to at least this number of next cities
	uint32 get_factory_worker_minimum_towns() const { return factory_worker_minimum_towns; }
	void set_factory_worker_minimum_towns(uint32 n) { factory_worker_minimum_towns = n; }

	// any factory will be connected to not more than this number of next cities
	uint32 get_factory_worker_maximum_towns() const { return factory_worker_maximum_towns; }
	void set_factory_worker_maximum_towns(uint32 n) { factory_worker_maximum_towns = n; }

	// Knightly : number of periods for averaging the amount of arrived pax/mail at factories
	uint16 get_factory_arrival_periods() const { return factory_arrival_periods; }

	// Knightly : whether factory pax/mail demands are enforced
	bool get_factory_enforce_demand() const { return factory_enforce_demand; }

	// disallow using obsolete vehicles in depot
	bool get_allow_buying_obsolete_vehicles() const { return allow_buying_obsolete_vehicles; }

	// forest stuff
	uint8 get_forest_base_size() const { return forest_base_size; }
	uint8 get_forest_map_size_divisor() const { return forest_map_size_divisor; }
	uint8 get_forest_count_divisor() const { return forest_count_divisor; }
	uint16 get_forest_inverse_spare_tree_density() const { return forest_inverse_spare_tree_density; }
	uint8 get_max_no_of_trees_on_square() const { return max_no_of_trees_on_square; }
	uint16 get_tree_climates() const { return tree_climates; }
	uint16 get_no_tree_climates() const { return no_tree_climates; }
	bool get_no_trees() const { return no_trees; }
	void set_no_trees(bool b) { no_trees = b; }

	uint32 get_industry_increase_every() const { return industry_increase; }
	uint32 get_city_isolation_factor() const { return city_isolation_factor; }
	void set_industry_increase_every( uint32 n ) { industry_increase = n; }

	sint16 get_used_vehicle_reduction() const { return used_vehicle_reduction; }

	void set_default_player_color( spieler_t *sp ) const;

	// usually only used in network mode => no need to set them!
	uint32 get_random_counter() const { return random_counter; }
	uint32 get_frames_per_second() const { return frames_per_second; }
	uint32 get_frames_per_step() const { return frames_per_step; }

	uint16 get_max_walking_distance() const { return max_walking_distance; }
	bool get_quick_city_growth() const { return quick_city_growth; }
	void set_quick_city_growth(bool value) { quick_city_growth = value; }
	bool get_assume_everywhere_connected_by_road() const { return assume_everywhere_connected_by_road; }
	void set_assume_everywhere_connected_by_road(bool value) { assume_everywhere_connected_by_road = value; }

	uint32 get_city_threshold_size() const { return city_threshold_size; }
	void set_city_threshold_size(uint32 value) { city_threshold_size = value; }
	uint32 get_capital_threshold_size() const { return capital_threshold_size; }
	void set_capital_threshold_size(uint32 value) { capital_threshold_size = value; }
	uint32 get_max_small_city_size() const { return max_small_city_size; }
	void set_max_small_city_size(uint32 value) { max_small_city_size = value; }
	uint32 get_max_city_size() const { return max_city_size; }
	void set_max_city_size(uint32 value) { max_city_size = value; }

	uint16 get_default_increase_maintenance_after_years(waytype_t wtype) const { return default_increase_maintenance_after_years[wtype]; }
	void set_default_increase_maintenance_after_years(waytype_t wtype, uint16 value) { default_increase_maintenance_after_years[wtype] = value; }
	uint32 get_server_frames_ahead() const { return server_frames_ahead; }


	uint8 get_spacing_shift_mode() const { return spacing_shift_mode; }
	void set_spacing_shift_mode(uint8 s) { spacing_shift_mode = s; }

	sint16 get_spacing_shift_divisor() const { return spacing_shift_divisor; }
	void set_spacing_shift_divisor(sint16 s) { spacing_shift_divisor = s; }

	livery_scheme_t* get_livery_scheme(uint16 index) { return !livery_schemes.empty() ? livery_schemes.get_element(index) : NULL; } 
	vector_tpl<livery_scheme_t*>* get_livery_schemes() { return &livery_schemes; }

	bool get_allow_routing_on_foot() const { return allow_routing_on_foot; }
	void set_allow_routing_on_foot(bool value); 

	bool is_drive_left() const { return drive_on_left; }
	bool is_signals_left() const { return signals_on_left; }

	uint16 get_min_wait_airport() const { return min_wait_airport; }
	void set_min_wait_airport(uint16 value) { min_wait_airport = value; }

	sint32 get_way_toll_runningcost_percentage() const { return way_toll_runningcost_percentage; }
	sint32 get_way_toll_waycost_percentage() const { return way_toll_waycost_percentage; }
	sint32 get_way_toll_revenue_percentage() const { return way_toll_revenue_percentage; }
	sint32 get_seaport_toll_revenue_percentage() const { return seaport_toll_revenue_percentage; }
	sint32 get_airport_toll_revenue_percentage() const { return airport_toll_revenue_percentage; }

	bool get_toll_free_public_roads() const { return toll_free_public_roads; }

	bool get_allow_making_public() const { return allow_making_public; }

	sint64 get_private_car_toll_per_tile() const { return private_car_toll_per_tile; }

	bool get_towns_adopt_player_roads() const { return towns_adopt_player_roads; }

#ifndef NETTOOL
	float32e8_t get_simtime_factor() const { return simtime_factor; }
	float32e8_t meters_to_steps(const float32e8_t &meters) const { return steps_per_meter * meters; }
	float32e8_t steps_to_meters(const float32e8_t &steps) const { return meters_per_step * steps; }
	float32e8_t ticks_to_seconds(sint32 delta_t) const { return seconds_per_tick * delta_t; }
#endif
	uint8 get_max_elevated_way_building_level() const { return max_elevated_way_building_level; }
	void set_max_elevated_way_building_level(uint8 value) { max_elevated_way_building_level = value; }
};

#endif 
