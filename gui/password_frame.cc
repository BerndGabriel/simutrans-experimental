/*
 * Copyright (c) 1997 - 2001 Hansjörg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#include <string.h>
#include "../simdebug.h"
#include "../simwerkz.h"
#include "../simwin.h"
#include "../simworld.h"

#include "../utils/sha1.h"

#include "components/list_button.h"
#include "password_frame.h"

#define DIALOG_WIDTH (360)


password_frame_t::password_frame_t( spieler_t *sp ) :
	gui_frame_t("Enter Password",sp)
{
	this->sp = sp;

	fnlabel.set_pos (koord(10,12));
	fnlabel.set_text( "Password" );
	add_komponente(&fnlabel);

	// Input box for game name
	ibuf[0] = 0;
	input.set_text(ibuf, 1024);
	input.add_listener(this);
	input.set_pos(koord(75,8));
	input.set_groesse(koord(DIALOG_WIDTH-75-10-10, 14));
	add_komponente(&input);

	set_focus( &input );

	set_fenstergroesse(koord(DIALOG_WIDTH, 40));
}




/**
 * This method is called if an action is triggered
 * @author Hj. Malthaner
 */
bool password_frame_t::action_triggered( gui_action_creator_t *komp,value_t /* */)
{
	if(komp == &input) {
		// Enter-Key pressed
		// test for matching password to unlock
		SHA1 sha1;
		sha1.Input( input.get_text(), strlen( input.get_text() ) );
		uint8 hash[20];
		MEMZERO(hash);
		sha1.Result( hash );
		/* if current active player is player 1 and this is unlocked, he may reset passwords
		 * otherwise you need the valid previous password
		 */
		if(  !sp->is_locked()  ||  (sp->get_welt()->get_active_player_nr()==1  &&  !sp->get_welt()->get_spieler(1)->is_locked())   ) {
			// set this to world
			sp->get_welt()->set_player_password_hash( sp->get_player_nr(), hash );
			// and change player password
			werkzeug_t *w = create_tool( WKZ_PWDHASH_TOOL | SIMPLE_TOOL );
			cbuffer_t buf(512);
			for(  int i=0;  i<20;  i++  ) {
				buf.printf( "%02X", hash[i] );
			}
			w->set_default_param(buf);
			sp->get_welt()->set_werkzeug( w, sp );
			delete w;
		}
		else {
			// set this to world to unlock
			sp->get_welt()->set_player_password_hash( sp->get_player_nr(), hash );
			sp->set_unlock( hash );
		}
		// always destroy window
		destroy_win(this);
	}
	return true;
}
