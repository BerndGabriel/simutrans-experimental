/*
 * Copyright (c) 1997 - 2001 Hj. Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */ 

#include "depot_frame.h"

#include <stdio.h>

#include "../simunits.h"
#include "../simconvoi.h"
#include "../simdepot.h"

#include "../simcolor.h"
#include "../simdebug.h"
#include "../simgraph.h"
#include "../simline.h"
#include "../simlinemgmt.h"
#include "../vehicle/simvehikel.h"
#include "../simmenu.h"
#include "../simtools.h"

#include "../besch/haus_besch.h"

#include "../simworld.h"
#include "../simwin.h"

#include "fahrplan_gui.h"
#include "line_management_gui.h"
#include "line_item.h"
#include "messagebox.h"

#include "../dataobj/fahrplan.h"
#include "../dataobj/translator.h"

#include "../player/simplay.h"

#include "../utils/simstring.h"
#include "../utils/cbuffer_t.h"

#include "../boden/wege/weg.h"

#define CREDIT_MESSAGE "That would exceed\nyour credit limit."

char depot_frame_t::no_line_text[128];	// contains the current translation of "<no line>"


depot_frame_t::depot_frame_t(depot_t* depot) :
	gui_frame_t( translator::translate(depot->get_name()), depot->get_besitzer()),
	depot(depot),
	icnv(depot->convoi_count()-1),
	lb_convois(NULL, COL_BLACK, gui_label_t::left),
	lb_convoi_value(NULL, COL_BLACK, gui_label_t::right),
	lb_convoi_line(NULL, COL_BLACK, gui_label_t::left),
	lb_traction_types(NULL, COL_BLACK, gui_label_t::left),
	convoy_assembler(get_welt(), depot->get_wegtyp(), depot->get_player_nr(), check_way_electrified(true) )
{
DBG_DEBUG("depot_frame_t::depot_frame_t()","get_max_convoi_length()=%i",depot->get_max_convoi_length());
	selected_line = depot->get_selected_line();
	strcpy(no_line_text, translator::translate("<no line>"));

	/*
	 * [CONVOY ASSEMBLER]
	 */
	convoy_assembler.set_depot_frame(this);
	convoy_assembler.add_listener(this);
	update_convoy();

	/*
	 * [SELECT]:
	 */
	add_komponente(&lb_convois);

	bt_prev.set_typ(button_t::arrowleft);
	bt_prev.add_listener(this);
	add_komponente(&bt_prev);

	inp_name.add_listener(this);
	add_komponente(&inp_name);

	bt_next.set_typ(button_t::arrowright);
	bt_next.add_listener(this);
	add_komponente(&bt_next);

	/*
	* [SELECT ROUTE]:
	*/
	line_selector.add_listener(this);
	line_selector.set_highlight_color(depot->get_besitzer()->get_player_color1() + 1);
	add_komponente(&line_selector);
	depot->get_besitzer()->simlinemgmt.sort_lines();

	add_komponente(&lb_convoi_value);
	add_komponente(&lb_convoi_line);
	add_komponente(&lb_traction_types);

	/*
	* [ACTIONS]
	*/
	bt_start.set_typ(button_t::roundbox);
	bt_start.add_listener(this);
	bt_start.set_tooltip("Start the selected vehicle(s)");
	add_komponente(&bt_start);

	bt_schedule.set_typ(button_t::roundbox);
	bt_schedule.add_listener(this);
	bt_schedule.set_tooltip("Give the selected vehicle(s) an individual schedule");
	add_komponente(&bt_schedule);

	bt_destroy.set_typ(button_t::roundbox);
	bt_destroy.add_listener(this);
	bt_destroy.set_tooltip("Move the selected vehicle(s) back to the depot");
	add_komponente(&bt_destroy);

	bt_sell.set_typ(button_t::roundbox);
	bt_sell.add_listener(this);
	bt_sell.set_tooltip("Sell the selected vehicle(s)");
	add_komponente(&bt_sell);

	/*
	* new route management buttons
	*/
	bt_new_line.set_typ(button_t::roundbox);
	bt_new_line.add_listener(this);
	bt_new_line.set_tooltip("Lines are used to manage groups of vehicles");
	add_komponente(&bt_new_line);

	bt_apply_line.set_typ(button_t::roundbox);
	bt_apply_line.add_listener(this);
	bt_apply_line.set_tooltip("Add the selected vehicle(s) to the selected line");
	add_komponente(&bt_apply_line);

	bt_change_line.set_typ(button_t::roundbox);
	bt_change_line.add_listener(this);
	bt_change_line.set_tooltip("Modify the selected line");
	add_komponente(&bt_change_line);

	bt_copy_convoi.set_typ(button_t::roundbox);
	bt_copy_convoi.add_listener(this);
	bt_copy_convoi.set_tooltip("Copy the selected convoi and its schedule or line");
	add_komponente(&bt_copy_convoi);

	koord gr = koord(0,0);
	layout(&gr);
	update_data();
	gui_frame_t::set_fenstergroesse(gr);

	// text will be translated by ourselves (after update data)!
	lb_convois.set_text_pointer(txt_convois);
	lb_convoi_value.set_text_pointer(txt_convoi_value);
	lb_convoi_line.set_text_pointer(txt_convoi_line);
	lb_traction_types.set_text_pointer(txt_traction_types);

	check_way_electrified();
	add_komponente(&img_bolt);

	add_komponente(&convoy_assembler);

	if(depot->get_tile()->get_besch()->get_enabled() == 0)
	{
		sprintf(txt_traction_types, "%s", translator::translate("Unpowered vehicles only"));
	}
	else if(depot->get_tile()->get_besch()->get_enabled() == 255)
	{
		sprintf(txt_traction_types, "%s", translator::translate("All traction types"));
	}
	else
	{
		uint8 shifter;
		bool first = true;
		uint8 n = 0;
		for(uint8 i = 0; i < 8; i ++)
		{
			shifter = 1 << i;
			if((shifter & depot->get_tile()->get_besch()->get_enabled()))
			{
				if(first)
				{
					first = false;
				}
				else
				{
					n += sprintf(txt_traction_types + n, ", ");
				}
				n += sprintf(txt_traction_types + n, "%s", translator::translate(vehikel_besch_t::get_engine_type((vehikel_besch_t::engine_t)i)));
			}
		}
	}

	// Hajo: Trigger layouting
	set_resizemode(diagonal_resize);
}

depot_frame_t::~depot_frame_t()
{
	// change convoy name if necessary
	rename_convoy( depot->get_convoi(icnv) );
}


// returns position of depot on the map
koord3d depot_frame_t::get_weltpos()
{
	return depot->get_pos();
}


void depot_frame_t::layout(koord *gr)
{
	koord fgr = (gr!=NULL)? *gr : get_fenstergroesse();

	/*
	* These parameter are adjusted to resolution.
	* - Some extra space looks nicer.
	grid.x = depot->get_x_grid() * get_base_tile_raster_width() / 64 + 4;
	grid.y = depot->get_y_grid() * get_base_tile_raster_width() / 64 + 6;
	placement.x = depot->get_x_placement() * get_base_tile_raster_width() / 64 + 2;
	placement.y = depot->get_y_placement() * get_base_tile_raster_width() / 64 + 2;
	grid_dx = depot->get_x_grid() * get_base_tile_raster_width() / 64 / 2;
	placement_dx = depot->get_x_grid() * get_base_tile_raster_width() / 64 / 4;

	/*
	*	Dialog format:
	*
	*	Main structure are these parts from top to bottom:
	*
	*	    [SELECT]		convoi-selector
	*	    [CONVOI]		current convoi (*)
	*	    [ACTIONS]		convoi action buttons
	*	    [PANEL]		vehicle panel (*)
	*	    [VINFO]		vehicle infotext (*)
	*
	*	(*) In CONVOI ASSEMBLER
	*
	*
	*	Structure of [SELECT] is:
	*
	*	    [Info]
	*	    [PREV][LABEL][NEXT]
	*
	*  PREV and NEXT are small buttons - Label is adjusted to total width.
	*/
	int SELECT_HEIGHT = 14;

	/*
	*	Structure of [ACTIONS] is a row of buttons:
	*
	*	    [Start][Schedule][Destroy][Sell]
	*      [new Route][change Route][delete Route]
	*/
	int ABUTTON_WIDTH = 128;
	int ABUTTON_HEIGHT = 14;
	int ACTIONS_WIDTH = 2+4*(ABUTTON_WIDTH+2);
	int ACTIONS_HEIGHT = ABUTTON_HEIGHT + ABUTTON_HEIGHT; // @author hsiegeln: added "+ ABUTTON_HEIGHT"
	convoy_assembler.set_convoy_tabs_skip(ACTIONS_HEIGHT);

	/*
	* Total width is the max from [CONVOI] and [ACTIONS] width.
	*/
	int MIN_DEPOT_FRAME_WIDTH = min((float)display_get_width() *0.7F, max(convoy_assembler.get_convoy_image_width(), ACTIONS_WIDTH));
	int DEPOT_FRAME_WIDTH = min((float)display_get_width() * 0.7F, max(fgr.x,max(convoy_assembler.get_convoy_image_width(), ACTIONS_WIDTH)));

	/*
	*	Now we can do the first vertical adjustement:
	*/
	int SELECT_VSTART = 16;
	int ASSEMBLER_VSTART = SELECT_VSTART + SELECT_HEIGHT + LINESPACE;
	int ACTIONS_VSTART = ASSEMBLER_VSTART + convoy_assembler.get_convoy_height();

	/*
	* Now we determine the row/col layout for the panel and the total panel
	* size.
	* build_vehicle_lists() fills loks_vec and waggon_vec.
	* Total width will be expanded to match completo columns in panel.
	*/
	convoy_assembler.set_panel_rows(gr  &&  gr->y==0?-1:fgr.y-ASSEMBLER_VSTART);

	/*
	 *	Now we can do the complete vertical adjustement:
	 */
	int TOTAL_HEIGHT = min((float)display_get_height() *0.9F, ASSEMBLER_VSTART + convoy_assembler.get_height());
	int MIN_TOTAL_HEIGHT =  min((float)display_get_height() *0.9F, ASSEMBLER_VSTART + convoy_assembler.get_min_height());

	/*
	* DONE with layout planning - now build everything.
	*/
	set_min_windowsize(koord(MIN_DEPOT_FRAME_WIDTH, MIN_TOTAL_HEIGHT));
	if(fgr.x<DEPOT_FRAME_WIDTH) {
		gui_frame_t::set_fenstergroesse(koord(MIN_DEPOT_FRAME_WIDTH, max(fgr.y,MIN_TOTAL_HEIGHT) ));
	}
	if(gr  &&  gr->x==0) {
		gr->x = DEPOT_FRAME_WIDTH;
	}
	if(gr  &&  gr->y==0) {
		gr->y = TOTAL_HEIGHT;
	}

	/*
	 * [SELECT]:
	 */
	lb_convois.set_pos(koord(4, SELECT_VSTART - 10));

	bt_prev.set_pos(koord(3, SELECT_VSTART + 2));

	inp_name.set_pos(koord(3+12, SELECT_VSTART));
	inp_name.set_groesse(koord(DEPOT_FRAME_WIDTH - 26-3, 14));

	bt_next.set_pos(koord(DEPOT_FRAME_WIDTH - 12, SELECT_VSTART + 2));

	/*
	 * [SELECT ROUTE]:
	 * @author hsiegeln
	 */
	line_selector.set_pos(koord(3, SELECT_VSTART + 14));
	line_selector.set_groesse(koord(DEPOT_FRAME_WIDTH - 3, 14));
	line_selector.set_max_size(koord(DEPOT_FRAME_WIDTH - 3, LINESPACE*13+2+16));

	/*
	 * [CONVOI]
	 */
	convoy_assembler.set_pos(koord(0,ASSEMBLER_VSTART));
	convoy_assembler.set_groesse(koord(DEPOT_FRAME_WIDTH,convoy_assembler.get_height()));
	convoy_assembler.layout();

	lb_convoi_value.set_pos(koord(DEPOT_FRAME_WIDTH-10, ASSEMBLER_VSTART + convoy_assembler.get_convoy_image_height()));
	lb_convoi_line.set_pos(koord(4, ASSEMBLER_VSTART + convoy_assembler.get_convoy_image_height() + LINESPACE * 2));
	lb_traction_types.set_pos(koord(4, ACTIONS_VSTART + (ABUTTON_HEIGHT * 2)));
 

	/*
	 * [ACTIONS]
	 */
	bt_start.set_pos(koord(2, ACTIONS_VSTART));
	bt_start.set_groesse(koord(DEPOT_FRAME_WIDTH/4-2, ABUTTON_HEIGHT));
	bt_start.set_text("Start");

	bt_schedule.set_pos(koord(DEPOT_FRAME_WIDTH/4+2, ACTIONS_VSTART));
	bt_schedule.set_groesse(koord(DEPOT_FRAME_WIDTH*2/4-DEPOT_FRAME_WIDTH/4-3, ABUTTON_HEIGHT));
	bt_schedule.set_text("Fahrplan");

	bt_destroy.set_pos(koord(DEPOT_FRAME_WIDTH*2/4+1, ACTIONS_VSTART));
	bt_destroy.set_groesse(koord(DEPOT_FRAME_WIDTH*3/4-DEPOT_FRAME_WIDTH*2/4-2, ABUTTON_HEIGHT));
	bt_destroy.set_text("Aufloesen");

	bt_sell.set_pos(koord(DEPOT_FRAME_WIDTH*3/4+1, ACTIONS_VSTART));
	bt_sell.set_groesse(koord(DEPOT_FRAME_WIDTH-DEPOT_FRAME_WIDTH*3/4-3, ABUTTON_HEIGHT));
	bt_sell.set_text("Verkauf");

	/*
	 * ACTIONS for new route management buttons
	 * @author hsiegeln
	 */
	bt_new_line.set_pos(koord(2, ACTIONS_VSTART+ABUTTON_HEIGHT));
	bt_new_line.set_groesse(koord(DEPOT_FRAME_WIDTH/4-2, ABUTTON_HEIGHT));
	bt_new_line.set_text("New Line");

	bt_apply_line.set_pos(koord(DEPOT_FRAME_WIDTH/4+2, ACTIONS_VSTART+ABUTTON_HEIGHT));
	bt_apply_line.set_groesse(koord(DEPOT_FRAME_WIDTH*2/4-3-DEPOT_FRAME_WIDTH/4, ABUTTON_HEIGHT));
	bt_apply_line.set_text("Apply Line");

	bt_change_line.set_pos(koord(DEPOT_FRAME_WIDTH*2/4+1, ACTIONS_VSTART+ABUTTON_HEIGHT));
	bt_change_line.set_groesse(koord(DEPOT_FRAME_WIDTH*3/4-2-DEPOT_FRAME_WIDTH*2/4, ABUTTON_HEIGHT));
	bt_change_line.set_text("Update Line");

	bt_copy_convoi.set_pos(koord(DEPOT_FRAME_WIDTH*3/4+1, ACTIONS_VSTART+ABUTTON_HEIGHT));
	bt_copy_convoi.set_groesse(koord(DEPOT_FRAME_WIDTH-DEPOT_FRAME_WIDTH*3/4-3, ABUTTON_HEIGHT));
	bt_copy_convoi.set_text("Copy Convoi");

	const uint8 margin = 4;
	img_bolt.set_pos(koord(get_fenstergroesse().x-skinverwaltung_t::electricity->get_bild(0)->get_pic()->w-margin,margin));
}


void depot_frame_t::set_fenstergroesse( koord gr )
{
	koord g=gr;
	layout(&g);
	update_data();
	gui_frame_t::set_fenstergroesse(gr);
}

void depot_frame_t::activate_convoi( convoihandle_t c )
{
	// deselect ...
	icnv = -1;
	for(  uint i=0;  i<depot->convoi_count();  i++  ) {
		if(  c==depot->get_convoi(i)  ) {
			icnv = i;
			break;
		}
	}
	build_vehicle_lists();
}


static void get_line_list(const depot_t* depot, vector_tpl<linehandle_t>* lines)
{
	depot->get_besitzer()->simlinemgmt.get_lines(depot->get_line_type(), lines);
}


/*
* Reset counts and check for valid vehicles
*/
void depot_frame_t::update_data()
{
	switch(depot->convoi_count()) {
		case 0:
			tstrncpy(txt_convois, translator::translate("no convois"), lengthof(txt_convois));
			break;
		case 1:
			if(icnv == -1) {
				tstrncpy( txt_convois, translator::translate("1 convoi"), lengthof(txt_convois) );
			}
			else {
				sprintf(txt_convois, translator::translate("convoi %d of %d"), icnv + 1, depot->convoi_count());
			}
			break;
		default:
			if(icnv == -1) {
				sprintf(txt_convois, translator::translate("%d convois"), depot->convoi_count());
			}
			else {
				sprintf(txt_convois, translator::translate("convoi %d of %d"), icnv + 1, depot->convoi_count());
			}
			break;
	}

	/*
	* Reset counts and check for valid vehicles
	*/
	convoihandle_t cnv = depot->get_convoi(icnv);

	// if select convoy is changed -> apply name changes, as well as reset text buffers and text input
	if(  cnv!=prev_cnv  ) {
		rename_convoy( prev_cnv );
		reset_convoy_name( cnv );
		prev_cnv = cnv;
	}

	// update the line selector
	selected_line = depot->get_selected_line();
	line_selector.clear_elements();
	line_selector.append_element( new gui_scrolled_list_t::const_text_scrollitem_t( no_line_text, COL_BLACK ) );
	if(!selected_line.is_bound()) {
		line_selector.set_selection( 0 );
	}

	// check all matching lines
	vector_tpl<linehandle_t> lines;
	get_line_list(depot, &lines);
	FOR(vector_tpl<linehandle_t>, const line, lines) {
		line_selector.append_element( new line_scrollitem_t(line) );
		if(line==selected_line) {
			line_selector.set_selection( line_selector.count_elements()-1 );
		}
	}
	convoy_assembler.update_data();
}

void depot_frame_t::reset_convoy_name(convoihandle_t cnv)
{
	// reset convoy name only if the convoy is currently selected
	if(  cnv.is_bound()  &&  cnv==depot->get_convoi(icnv)  ) {
		tstrncpy(txt_old_cnv_name, cnv->get_name(), lengthof(txt_old_cnv_name));
		tstrncpy(txt_cnv_name, cnv->get_name(), lengthof(txt_cnv_name));
		inp_name.set_text(txt_cnv_name, lengthof(txt_cnv_name));
	}
}


void depot_frame_t::rename_convoy(convoihandle_t cnv)
{
	if(  cnv.is_bound()  ) {
		const char *t = inp_name.get_text();
		// only change if old name and current name are the same
		// otherwise some unintended undo if renaming would occur
		if(  t  &&  t[0]  &&  strcmp(t, cnv->get_name())  &&  strcmp(txt_old_cnv_name, cnv->get_name())==0  ) {
			// text changed => call tool
			cbuffer_t buf;
			buf.printf( "c%u,%s", cnv.get_id(), t );
			werkzeug_t *w = create_tool( WKZ_RENAME_TOOL | SIMPLE_TOOL );
			w->set_default_param( buf );
			cnv->get_welt()->set_werkzeug( w, cnv->get_besitzer() );
			// since init always returns false, it is safe to delete immediately
			delete w;
			// do not trigger this command again
			tstrncpy(txt_old_cnv_name, t, lengthof(txt_old_cnv_name));
		}
	}
}

bool depot_frame_t::action_triggered( gui_action_creator_t *komp,value_t p)
{
	convoihandle_t cnv = depot->get_convoi(icnv);
	rename_convoy( cnv );

	if(komp != NULL) {	// message from outside!
		if(komp == &bt_start) {
			if(  cnv.is_bound()  ) {
				//first: close schedule (will update schedule on clients)
				destroy_win( (long)cnv->get_schedule() );
				// only then call the tool to start
				depot->call_depot_tool( 'b', cnv, NULL );
				update_convoy();
			}
		} else if(komp == &bt_schedule) {
			fahrplaneingabe();
			return true;
		} else if(komp == &bt_destroy) {
			depot->call_depot_tool( 'd', cnv, NULL );
			update_convoy();
		} else if(komp == &bt_sell) {
			depot->call_depot_tool( 'v', cnv, NULL );
			update_convoy();
		} else if(komp == &inp_name) {
			return true;	// already call rename_convoy() above
		} else if(komp == &bt_next) {
			if(++icnv == (int)depot->convoi_count()) {
				icnv = -1;
			}
			update_convoy();
		} else if(komp == &bt_prev) {
			if(icnv-- == -1) {
				icnv = depot->convoi_count() - 1;
			}
			update_convoy();
		} else if(komp == &bt_new_line) {
			depot->call_depot_tool( 'l', convoihandle_t(), NULL );
			return true;
		} else if(komp == &bt_change_line) {
			if(selected_line.is_bound()) {
				create_win(new line_management_gui_t(selected_line, depot->get_besitzer()), w_info, (long)selected_line.get_rep() );
			}
			return true;
		} else if(komp == &bt_copy_convoi) {
			if(  convoihandle_t::is_exhausted()  ) {
				create_win( new news_img("Convoi handles exhausted!"), w_time_delete, magic_none);
			}
			else {
				depot->call_depot_tool( 'c', cnv, NULL);
				update_convoy();
			}
			return true;
		}
		else if(komp == &bt_apply_line) {
			apply_line();
		} else if(komp == &line_selector) {
			int selection = p.i;
//DBG_MESSAGE("depot_frame_t::action_triggered()","line selection=%i",selection);
			if(  (unsigned)(selection-1)<(unsigned)line_selector.count_elements()  ) {
				vector_tpl<linehandle_t> lines;
				get_line_list(depot, &lines);
				selected_line = lines[selection - 1];
				depot->set_selected_line(selected_line);
			}
			else {
				// remove line
				selected_line = linehandle_t();
				depot->set_selected_line(selected_line);
				line_selector.set_selection( 0 );
			}
		}
		else {
			return false;
		}
		convoy_assembler.build_vehicle_lists();
	}
	update_data();
	layout(NULL);
	return true;
}


bool depot_frame_t::infowin_event(const event_t *ev)
{
	if(ev->ev_code!=WIN_CLOSE  &&  get_welt()->get_active_player() != depot->get_besitzer()) {
		destroy_win(this);
		return true;
	}

	const bool swallowed = gui_frame_t::infowin_event(ev);

	if(IS_WINDOW_CHOOSE_NEXT(ev)) {

		bool dir = (ev->ev_code==NEXT_WINDOW);
		depot_t *next_dep = depot_t::find_depot( depot->get_pos(), depot->get_typ(), depot->get_besitzer(), dir == NEXT_WINDOW );
		if(next_dep == NULL) {
			if(dir == NEXT_WINDOW) {
				// check the next from start of map
				next_dep = depot_t::find_depot( koord3d(-1,-1,0), depot->get_typ(), depot->get_besitzer(), true );
			}
			else {
				// respecive end of map
				next_dep = depot_t::find_depot( koord3d(8192,8192,127), depot->get_typ(), depot->get_besitzer(), false );
			}
		}

		if(next_dep  &&  next_dep!=this->depot) {
			/**
			 * Replace our depot_frame_t with a new at the same position.
			 * Volker Meyer
			 */
			int x = win_get_posx(this);
			int y = win_get_posy(this);
			destroy_win( this );

			next_dep->zeige_info();
			win_set_pos( win_get_magic((long)next_dep), x, y );
			get_welt()->change_world_position(next_dep->get_pos());
		}
		else {
			// recenter on current depot
			get_welt()->change_world_position(depot->get_pos());
		}

		return true;

	} else if(IS_WINDOW_REZOOM(ev)) {
		koord gr = get_fenstergroesse();
		set_fenstergroesse(gr);
	} else if(ev->ev_class == INFOWIN && ev->ev_code == WIN_OPEN) {
		convoy_assembler.build_vehicle_lists();
		update_data();
		layout(NULL);
	}
	else {
		if(IS_LEFTCLICK(ev) &&  !line_selector.getroffen(ev->cx, ev->cy-16)) {
			// close combo box; we must do it ourselves, since the box does not receive outside events ...
			line_selector.close_box();
		}
	}

	return swallowed;
}



void depot_frame_t::zeichnen(koord pos, koord groesse)
{
	if (get_welt()->get_active_player() != depot->get_besitzer()) {
		destroy_win(this);
		return;
	}

	convoihandle_t cnv = depot->get_convoi(icnv);
	// check for data inconsistencies (can happen with withdraw-all and vehicle in depot)
	const vector_tpl<gui_image_list_t::image_data_t>* convoi_pics = convoy_assembler.get_convoi_pics();
	if(!cnv.is_bound() && convoi_pics->get_count() > 0)
	{
		icnv=0;
		update_data();
		cnv = depot->get_convoi(icnv);
	}

	if(cnv.is_bound()) {
		if(cnv->get_vehikel_anzahl() > 0) {
			char number[64];
			money_to_string( number, cnv->calc_restwert()/100.0 );
			sprintf(txt_convoi_value, "%s %s", translator::translate("Restwert:"), number);
			// just recheck if schedules match
			if(  cnv->get_line().is_bound()  &&  cnv->get_line()->get_schedule()->ist_abgeschlossen()  ) {
				cnv->check_pending_updates();
				if(  !cnv->get_line()->get_schedule()->matches( get_welt(), cnv->get_schedule() )  ) {
					cnv->unset_line();
				}
			}
			if(  cnv->get_line().is_bound()  ) {
				sprintf(txt_convoi_line, "%s %s", translator::translate("Serves Line:"), cnv->get_line()->get_name());

			}
			else {
				sprintf(txt_convoi_line, "%s %s", translator::translate("Serves Line:"), no_line_text);
			}
		}
		else {
			*txt_convoi_value = '\0';
		}
	}
	else {
		static char empty[2] = "\0";
		inp_name.set_text( empty, 0);
		*txt_convoi_value = '\0';
		*txt_convoi_line = '\0';
	}

	gui_frame_t::zeichnen(pos, groesse);

	if(!cnv.is_bound()) {
		display_proportional_clip(pos.x+inp_name.get_pos().x+2, pos.y+inp_name.get_pos().y+18, translator::translate("new convoi"), ALIGN_LEFT, COL_GREY1, true);
	}
}

void depot_frame_t::apply_line()
{
	if(icnv > -1) {
		convoihandle_t cnv = depot->get_convoi(icnv);
		// if no convoi is selected, do nothing
		if (!cnv.is_bound()) {
			return;
		}

		if(selected_line.is_bound()) {
			// set new route only, a valid route is selected:
			char id[16];
			sprintf( id, "%i", selected_line.get_id() );
			cnv->call_convoi_tool( 'l', id );
		}
		else {
			// sometimes the user might wish to remove convoy from line
			// => we clear the schedule completely
			cnv->call_convoi_tool( 'g', "0|" );
		}
	}
}


void depot_frame_t::fahrplaneingabe()
{
	convoihandle_t cnv = depot->get_convoi(icnv);
	if(cnv.is_bound()  &&  cnv->get_vehikel_anzahl() > 0) {
		// this can happen locally, since any update of the schedule is done during closing window
		schedule_t *fpl = cnv->create_schedule();
		assert(fpl!=NULL);
		gui_frame_t *fplwin = win_get_magic((long)fpl);
		if(   fplwin==NULL  ) {
			cnv->open_schedule_window( get_welt()->get_active_player()==cnv->get_besitzer() );
		}
		else {
			top_win( fplwin );
		}
	}
	else {
		create_win( new news_img("Please choose vehicles first\n"), w_time_delete, magic_none);
	}
}

bool depot_frame_t::check_way_electrified(bool init)
{
	const waytype_t wt = depot->get_wegtyp();
	const weg_t *w = get_welt()->lookup(depot->get_pos())->get_weg(wt!=tram_wt ? wt : track_wt);
	const bool way_electrified = w ? w->is_electrified() : false;
	if(!init)
	{
		convoy_assembler.set_electrified( way_electrified );
	}
	if( way_electrified ) 
	{
		//img_bolt.set_image(skinverwaltung_t::electricity->get_bild_nr(0));
	}

	else
	{
		//img_bolt.set_image(IMG_LEER);
 	}

	return way_electrified;
}
