// Interface from Falaise
#include "TKReconstruct.h"

DPP_MODULE_REGISTRATION_IMPLEMENT(TKReconstruct, "TKReconstruct")

TKReconstruct::TKReconstruct() : dpp::base_module() 
{
    eventNo = 0;
    std::cout << "constructed!" << std::endl;
}

TKReconstruct::~TKReconstruct()
{ 
    if (this->is_initialized())
    {
        this->reset();
    }
}

void TKReconstruct::initialize( 
            const datatools::properties&   myConfig,
            datatools::service_manager&    flServices,
            dpp::module_handle_dict_type&  /*moduleDict*/
        ) 
{
    this->_set_initialized(true);
}

dpp::base_module::process_status TKReconstruct::process(datatools::things& workItem) 
{

    TKEvent* event = get_event_data(workItem);
    event->reconstruct_ML(0);
    //event->print();
    event->make_top_projection(2);
    
    eventNo++;
	delete event;
    return falaise::processing::status::PROCESS_OK;
}

void TKReconstruct::reset() 
{   
    this->_set_initialized(false);
}


TKEvent* TKReconstruct::get_event_data(datatools::things& workItem)
{
	TKEvent* event = new TKEvent(0, eventNo);

    if(workItem.has("CD"))
    {
        using namespace snemo::datamodel;

        snemo::datamodel::calibrated_data falaiseCDbank = workItem.get<calibrated_data>("CD");

        for ( auto &calohit : falaiseCDbank.calorimeter_hits() )
        {
        	int SWCR[4] = {-1,-1,-1,-1};
        	switch( calohit->get_geom_id().get_type() )
        	{
        	case 1302: 
        		SWCR[0] = calohit->get_geom_id().get(1);
        		SWCR[2] = calohit->get_geom_id().get(2);
        		SWCR[3] = calohit->get_geom_id().get(3);
        		break;
        	case 1232:
        		SWCR[0] = calohit->get_geom_id().get(1);
        		SWCR[1] = calohit->get_geom_id().get(2);
        		SWCR[2] = calohit->get_geom_id().get(3);
        		SWCR[3] = calohit->get_geom_id().get(4);
        		break;
        	case 1252:
        		SWCR[0] = calohit->get_geom_id().get(1);
        		SWCR[1] = calohit->get_geom_id().get(2);
        		SWCR[2] = calohit->get_geom_id().get(3);
        		break;
		}
		TKOMhit* OMhit = new TKOMhit(SWCR);		
        	OMhit->set_energy( calohit->get_energy() ); 
        	
		event->get_OM_hits().push_back( OMhit );
        }
        
        for ( auto &trhit : falaiseCDbank.tracker_hits() )
        {
		int SRL[3] = {trhit->get_side(), trhit->get_row(), trhit->get_layer()};
        	TKtrhit* hit = new TKtrhit(SRL);
        	
        	hit->set_r( trhit->get_r() );
        	hit->set_sigma_R( trhit->get_sigma_r() );
          	hit->set_h( trhit->get_z() );
          	hit->set_sigma_Z( trhit->get_sigma_z() );
        	
        	event->get_tr_hits().push_back( hit );
        }

    }
    return event;
}
