#ifndef TKRECONSTRUCT_H
#define TKRECONSTRUCT_H

// Interface from Falaise
#include "bayeux/dpp/base_module.h"
#include "bayeux/mctools/simulated_data.h"

#include "falaise/snemo/processing/module.h"
#include "falaise/snemo/datamodels/particle_track_data.h"
#include "falaise/snemo/datamodels/tracker_clustering_solution.h"
#include "falaise/snemo/datamodels/calibrated_data.h"

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
    ////////////////////////////////////////////////
    // Everything else is optional for your usecase
    TKEvent* get_event_data(datatools::things &workItem);

private:
    int eventNo;
    TKEvent* event;

    DPP_MODULE_REGISTRATION_INTERFACE(TKReconstruct);
};

#endif // TKRECONSTRUCT_H
