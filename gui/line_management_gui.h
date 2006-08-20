/*
 * line_management.h
 * part of the Simutrans project
 * @author hsiegeln
 * 01/12/2003
 */

 #include "fahrplan_gui.h"
 #include "components/gui_textinput.h"
 #include "ifc/action_listener.h"
 #include "gui_frame.h"

 class simline_t;

 class line_management_gui_t : public fahrplan_gui_t
 {
 	public:
 	    line_management_gui_t(karte_t *welt, simline_t *line, spieler_t *sp);
	    ~line_management_gui_t();
	    virtual const char * gib_name() const;
	    virtual void infowin_event(const event_t *ev);
	private:
	    simline_t * line;
	    karte_t * welt;
};
