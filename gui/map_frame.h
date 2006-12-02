/*
 * map_frame.h
 *
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project and may not be used
 * in other projects without written permission of the author.
 */

/*
 * [Mathew Hounsell] Min Size Button On Map Window 20030313
 */

#include "gui_frame.h"
#include "components/gui_scrollpane.h"
#include "components/action_listener.h"
#include "components/gui_button.h"
#include "components/gui_label.h"
#include "karte.h"

class karte_t;

/**
 * Reliefkartenfenster f�r Simutrans.
 *
 * @author Hj. Malthaner
 * @date 03-Mar-01
 * @version $Revision: 1.11 $
 */
class map_frame_t :
	public gui_frame_t,
	public action_listener_t
{
private:

  /**
   * This is kind of hack: we know there can only be one map frame
   * at atime,and we want to save the current size for the next object
   * so we use a static variable here.
   * @author Hj. Malthaner
   */
	static koord size;
	static koord screenpos;

	static uint8 legend_visible;
	static uint8 scale_visible;
	static uint8 directory_visible;

	  /**
	   * We need to keep track of trag/click events
	   * @author Hj. Malthaner
	   */
	bool is_dragging;

	gui_scrollpane_t scrolly;

	// position of the buttons
	int row, col;

	// buttons
	static const char map_type[MAX_MAP_TYPE][64];
	static const int map_type_color[MAX_MAP_TYPE];
	button_t filter_buttons[MAX_MAP_TYPE];
	bool is_filter_active[MAX_MAP_TYPE];

	button_t zoom_buttons[2];
	gui_label_t zoom_label;
	button_t b_rotate45;

	button_t b_show_legend;
	button_t b_show_scale;
	button_t b_show_directory;

public:

    /**
     * Manche Fenster haben einen Hilfetext assoziiert.
     * @return den Dateinamen f�r die Hilfe, oder NULL
     * @author Hj. Malthaner
     */
    const char * gib_hilfe_datei() const {return "map.txt";}

    /**
     * Does this window need a min size button in the title bar?
     * @return true if such a button is needed
     * @author Hj. Malthaner
     */
    bool has_min_sizer() const {return true;}

    /**
     * Konstruktor. Erzeugt alle notwendigen Subkomponenten.
     * @author Hj. Malthaner
     */
    map_frame_t(const karte_t *welt);
    ~map_frame_t() {}

    /**
     * Events werden hiermit an die GUI-Komponenten
     * gemeldet
     * @author Hj. Malthaner
     */
    void infowin_event(const event_t *ev);

    /**
     * Setzt die Fenstergroesse
     * @author (Mathew Hounsell)
     * @date   11-Mar-2003
     */
    void setze_fenstergroesse(koord groesse);

    /**
     * resize window in response to a resize event
     * @author Hj. Malthaner
     * @date   01-Jun-2002
     */
    void resize(const koord delta);

    /**
     * komponente neu zeichnen. Die �bergebenen Werte beziehen sich auf
     * das Fenster, d.h. es sind die Bildschirkoordinaten des Fensters
     * in dem die Komponente dargestellt wird.
     * @author Hj. Malthaner
     */
    void zeichnen(koord pos, koord gr);

    /**
     * This method is called if an action is triggered
     * @author Hj. Malthaner
     *
     * Returns true, if action is done and no more
     * components should be triggered.
     * V.Meyer
     */
    bool action_triggered(gui_komponente_t *komp, value_t extra);
};
