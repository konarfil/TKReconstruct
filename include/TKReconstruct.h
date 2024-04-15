#ifndef TKRECONSTRUCT_H
#define TKRECONSTRUCT_H

// Interface from Falaise
#include "bayeux/dpp/base_module.h"
#include "bayeux/mctools/simulated_data.h"

// Third party:
// - Bayeux/geomtools:
#include <bayeux/geomtools/line_3d.h>

#include "falaise/snemo/processing/module.h"
#include "falaise/snemo/datamodels/calibrated_data.h"
#include <falaise/snemo/datamodels/data_model.h>
#include <falaise/snemo/datamodels/event_header.h>
#include <falaise/snemo/datamodels/tracker_clustering_data.h>
#include "falaise/snemo/datamodels/tracker_clustering_solution.h"
#include <falaise/snemo/datamodels/tracker_trajectory_data.h>
#include <falaise/snemo/datamodels/tracker_trajectory_solution.h>
#include <falaise/snemo/datamodels/line_trajectory_pattern.h>
#include "falaise/snemo/datamodels/particle_track_data.h"

#include "TKEvent.h"

class TKReconstruct : public dpp::base_module
{
public:
    ////////////////////////////////////////////////
    // The following PUBLIC methods MUST be defined!
    // Default constructor
    TKReconstruct();

    // Default destructor
    virtual ~TKReconstruct();

    //! Configure the module
    virtual void initialize(
        const datatools::properties &myConfig,
        datatools::service_manager &flServices,
        dpp::module_handle_dict_type &what);

    //! Reset the module
    virtual void reset();

    // Process event
    virtual dpp::base_module::process_status process(datatools::things &workItem);


    TKEvent* get_event_data(datatools::things &workItem);
    void fill_TCD_bank(TKEvent* event, snemo::datamodel::calibrated_data& falaiseCDbank, snemo::datamodel::tracker_clustering_data& the_tracker_clustering_data);
    void fill_TTD_bank(TKEvent* event, snemo::datamodel::tracker_clustering_data& the_tracker_clustering_data, snemo::datamodel::tracker_trajectory_data& the_tracker_trajectory_data);
    
    void line_to_verteces(TKtrack* track, geomtools::line_3d & line_3d);

private:

    DPP_MODULE_REGISTRATION_INTERFACE(TKReconstruct);
};

#endif // TKRECONSTRUCT_H
