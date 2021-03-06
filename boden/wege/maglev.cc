#include "../../simtypes.h"
#include "../../bauer/wegbauer.h"
#include "../../besch/weg_besch.h"

#include "maglev.h"

const weg_besch_t *maglev_t::default_maglev=NULL;



/**
 * File loading constructor.
 * @author prissi
 */
maglev_t::maglev_t(loadsave_t *file) : schiene_t(maglev_wt)
{
	rdwr(file);
}


void maglev_t::rdwr(loadsave_t *file)
{
	schiene_t::rdwr(file);

	if(get_besch()->get_wtyp()!=maglev_wt) {
		int old_max_speed = get_max_speed();
		int old_max_axle_load = get_max_axle_load();
		const weg_besch_t *besch = wegbauer_t::weg_search( maglev_wt, (old_max_speed>0 ? old_max_speed : 120), (old_max_axle_load > 0 ? old_max_axle_load : 10), 0, (weg_t::system_type)((get_besch()->get_styp()==weg_t::type_elevated)*weg_t::type_elevated), 1);
		if (besch==NULL) {
			dbg->fatal("maglev_t::rwdr()", "No maglev way available");
		}
		dbg->warning("maglev_t::rwdr()", "Unknown way replaced by maglev %s (old_max_speed %i)", besch->get_name(), old_max_speed );
		set_besch(besch, file->get_experimental_version() >= 12);
		if(old_max_speed > 0) {
			set_max_speed(old_max_speed);
		}
		if(old_max_axle_load > 0)
		{
			set_max_axle_load(old_max_axle_load);
		}
	}
}
