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
	TKEvent* event = get_event_data(workItem); 	// event -> TKEvent
	event->reconstruct_ML(0);			// TKEvent -> TKtrack
	//event->print();
	//event->make_top_projection(2);
	//event->build_event();
	
	namespace snedm = snemo::datamodel;

	// Create or reset TCD bank
	auto & the_tracker_clustering_data = ::snedm::getOrAddToEvent<snedm::tracker_clustering_data>("TCD", workItem);
	the_tracker_clustering_data.clear();

	// Fill TKcluster data into TCD bank
        snedm::calibrated_data falaiseCDbank = workItem.get<snedm::calibrated_data>("CD");
	fill_TCD_bank(event, falaiseCDbank, the_tracker_clustering_data);


	// Create or reset TTD bank
	auto & the_tracker_trajectory_data = ::snedm::getOrAddToEvent<snedm::tracker_trajectory_data>("TTD", workItem);
	the_tracker_trajectory_data.clear();

	// Fill TKtrack data into TTD bank
	fill_TTD_bank(event, the_tracker_clustering_data, the_tracker_trajectory_data);

	/*
	// Create or reset PTD bank
	auto & the_particle_track_data = ::snedm::getOrAddToEvent<snedm::particle_track_data>("PTD", workItem);
	the_particle_track_data.clear();
	
	// Fill PTD bank
	fill_PTD_bank(event, the_tracker_clustering_data, the_tracker_trajectory_data, the_particle_track_data);
	*/

	delete event;
	eventNo++;
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
        	
        	if( trhit->get_r() == trhit->get_r() )
        	{
				hit->set_r( trhit->get_r() );        	
				hit->set_sigma_R( /*trhit->get_sigma_r()*/ 2.0 );
        	}
        	else
        	{
        		hit->set_r( -1 );        	
				hit->set_sigma_R( -1 );
        	}
          	hit->set_h( trhit->get_z() );
          	hit->set_sigma_Z( trhit->get_sigma_z() );
        	
        	/*
        	std::cout << "r: 	" << trhit->get_r() << std::endl;
        	std::cout << "sigma_r: 	" << trhit->get_sigma_r() << std::endl;
        	std::cout << "Z: 	" << trhit->get_z() << std::endl;
        	std::cout << "sigma_Z: 	" << trhit->get_sigma_z() << std::endl;
        	*/
        	event->get_tr_hits().push_back( hit );
        }

    }
    return event;
}

void TKReconstruct::fill_TCD_bank(TKEvent* event, snemo::datamodel::calibrated_data& falaiseCDbank, snemo::datamodel::tracker_clustering_data& the_tracker_clustering_data)
{
	namespace snedm = snemo::datamodel;
 
	auto htcs = datatools::make_handle<snedm::TrackerClusteringSolution>();
	the_tracker_clustering_data.push_back(htcs, true);
	the_tracker_clustering_data.get_default().set_solution_id(the_tracker_clustering_data.size() - 1);

	snedm::tracker_clustering_solution& clustering_solution = the_tracker_clustering_data.get_default();

	for(int i = 0; i < event->get_clusters().size(); i ++)
	{
		snedm::TrackerClusterHdl tch(new snedm::tracker_cluster);
	  	clustering_solution.get_clusters().push_back(tch);
		  
		snedm::TrackerClusterHdl& cluster_handle = clustering_solution.get_clusters().back();
		//cluster_handle->set_cluster_id(clustering_solution.get_clusters().size() - 1);
		cluster_handle->set_cluster_id(i);
		
		TKcluster* cluster = event->get_clusters()[i];
		for (int j = 0; j < cluster->get_tr_hits().size(); j++)
		{
			TKtrhit* TKhit = cluster->get_tr_hits()[j];
			int SRL[3] = {TKhit->get_SRL('S'), TKhit->get_SRL('R'), TKhit->get_SRL('L')};
			for ( auto &trhit : falaiseCDbank.tracker_hits() )
			{
				if(SRL[0] == trhit->get_side() && SRL[1] == trhit->get_row() && SRL[2] == trhit->get_layer())
				{
					cluster_handle->hits().push_back(trhit);				
				}
			}
			
		}		
	}

	return;
}

void TKReconstruct::fill_TTD_bank(TKEvent* event, snemo::datamodel::tracker_clustering_data& the_tracker_clustering_data, snemo::datamodel::tracker_trajectory_data& the_tracker_trajectory_data)
{	
	namespace snedm = snemo::datamodel;

	// Get cluster solutions:
	const snedm::TrackerClusteringSolutionHdlCollection & cluster_solutions = the_tracker_clustering_data.solutions();

	// Loop on all clustering solutions:
	for (const datatools::handle<snedm::tracker_clustering_solution> & a_cluster_solution : cluster_solutions) 
	{
		// Build a new trajectory solution for each clustering solution:
		auto a_trajectory_solution = datatools::make_handle<snedm::tracker_trajectory_solution>();
		the_tracker_trajectory_data.add_solution(a_trajectory_solution);
		a_trajectory_solution->set_solution_id(a_cluster_solution->get_solution_id());
		a_trajectory_solution->set_clustering_solution(a_cluster_solution);
		
		// Get clusters stored in the current tracker solution:
		const snedm::TrackerClusterHdlCollection & clusters = a_cluster_solution->get_clusters();
		
		// Loop over TKclusters
		for (int i = 0; i < event->get_clusters().size(); i ++)
		{
			TKcluster* cluster = event->get_clusters()[i];
			if(cluster->get_track() == nullptr) continue;
			
			// using the first hit for TKcluster <-> Falaise cluster identification 
			TKtrhit* TKhit = cluster->get_tr_hits()[0];
			int SRL[3] = {TKhit->get_SRL('S'), TKhit->get_SRL('R'), TKhit->get_SRL('L')};

			// matching Falaise clusters with the TKcluster
			for (const datatools::handle<snedm::tracker_cluster> & a_cluster : clusters) 
			{
				bool same_cluster = false;
				for ( auto &trhit : a_cluster->hits() )
				{
					if(SRL[0] == trhit->get_side() && SRL[1] == trhit->get_row() && SRL[2] == trhit->get_layer())
					{
						same_cluster = true;			
					}
				}
				
				if(!same_cluster) continue;
			
				snedm::TrackerTrajectoryHdlCollection Trajectories;
							
				// Create new 'tracker_trajectory' handle:  
				auto h_trajectory = datatools::make_handle<snedm::tracker_trajectory>();
				Trajectories.push_back(h_trajectory);

				// Create new 'tracker_pattern' handle:
				snemo::datamodel::TrajectoryPatternHdl h_pattern;
				auto line_pattern = new snedm::line_trajectory_pattern;
				h_pattern.reset(line_pattern);
				
				// Set cluster and pattern handle to tracker_trajectory:
				auto id = Trajectories.size();
				h_trajectory->set_id(id);
				h_trajectory->set_cluster_handle(a_cluster);
				h_trajectory->set_pattern_handle(h_pattern);
				
				/*double chi2 = std::pow(a_fit_solution.chi, 2);
				h_trajectory->get_fit_infos().set_chi2(chi2);
				h_trajectory->get_fit_infos().set_ndof(a_fit_solution.ndof);
				*/
				
				geomtools::line_3d & line_3d = line_pattern->get_segment();
				line_to_verteces(cluster->get_track(), line_3d);
				
				unsigned int thisTrajId = a_trajectory_solution->grab_best_trajectories().size();
				h_trajectory->set_id(thisTrajId);
				a_trajectory_solution->grab_trajectories().push_back(h_trajectory);
			
			} 
		
		}
	}
	
	return;
}


void TKReconstruct::fill_PTD_bank(TKEvent* event, snemo::datamodel::tracker_clustering_data& the_tracker_clustering_data, snemo::datamodel::tracker_trajectory_data& the_tracker_trajectory_data, snemo::datamodel::particle_track_data& the_particle_track_data)
{

	namespace snedm = snemo::datamodel;
	
	// Get trajectory solutions:
	const snedm::TrackerTrajectorySolutionHdlCollection & trajectory_solutions = the_tracker_trajectory_data.get_solutions();

	// Loop on all trajectory solutions:
	for (const datatools::handle<snedm::tracker_trajectory_solution> & a_trajectory_solution : trajectory_solutions) 
	{
		// Get traejctories stored in the current trajectory solution:
		const snedm::TrackerTrajectoryHdlCollection & trajectories = a_trajectory_solution->get_trajectories();
		
		for (const datatools::handle<snedm::tracker_trajectory> & a_trajectory : trajectories) 
		{
	
			// Create a handle of fake vertices :
			snedm::VertexHdl vertex_1 = datatools::make_handle<snedm::Vertex>();
			vertex_1->set_hit_id(0);
			//vertex_1->grab_geom_id().set_type(1102);      // "source_strip" geometry category
			//vertex_1->grab_geom_id().set_address(0, 23);  // module #0, source strip #23
			//vertex_1->set_category(snedm::VERTEX_CATEGORY_ON_SOURCE_FOIL);
			//vertex_1->set_from(snedm::VERTEX_FROM_LAST);
			vertex_1->set_extrapolation(snedm::VERTEX_EXTRAPOLATION_LINE);
			vertex_1->set_spot(geomtools::blur_spot(geomtools::blur_spot::dimension_three));
			vertex_1->get_spot().set_position(geomtools::vector_3d(1.0 * CLHEP::mm, 2.0 * CLHEP::mm, 0.0 * CLHEP::mm));
			//vertex_1->get_spot().set_x_error(0.5 * CLHEP::mm);
			//vertex_1->get_spot().set_y_error(0.5 * CLHEP::mm);
			//vertex_1->get_spot().set_z_error(0.5 * CLHEP::mm);

			snedm::VertexHdl vertex_2 = datatools::make_handle<snedm::Vertex>();
			vertex_2->set_hit_id(1);
			vertex_2->set_extrapolation(snedm::VERTEX_EXTRAPOLATION_LINE);
			vertex_2->set_spot(geomtools::blur_spot(geomtools::blur_spot::dimension_three));
			vertex_2->get_spot().set_position(geomtools::vector_3d(430.0 * CLHEP::mm, 20.0 * CLHEP::mm, 30.0 * CLHEP::mm));

			// Create the particle track :
			datatools::handle<snedm::particle_track> particle_track;
			particle_track.reset(new snedm::particle_track);
			particle_track.grab().set_track_id(0);
			particle_track.grab().set_charge(snedm::particle_track::CHARGE_UNDEFINED); 
			
			particle_track.grab().set_trajectory_handle(a_trajectory);
			particle_track.grab().get_vertices().push_back(vertex_1);
			particle_track.grab().get_vertices().push_back(vertex_2);

			// Particle track data bank :
			the_particle_track_data.insertParticle(particle_track);
		}
	}

	return;
}

void TKReconstruct::line_to_verteces(TKtrack* track, geomtools::line_3d & line_3d)
{
	line_3d.set_first(0.0 * CLHEP::mm, track->get_b() * CLHEP::mm, track->get_d() * CLHEP::mm);
	
	double x,y,z;
	
	//x = 435.0;
	x = 400.0;
	if( track->get_side() == 0 ) 
	{
		x = -x;
	}		
	y = track->get_a()*x + track->get_b();
	
	if( y > 2505.5 )
	{
		x = (2505.5-track->get_b())/track->get_a();
		y = track->get_a()*x + track->get_b();
		
	}
	else if( y < -2505.5 )
	{
		x = (-2505.5-track->get_b())/track->get_a();
		y = track->get_a()*x + track->get_b();
	}
	z = track->get_c()*x + track->get_d();
	
	if( z > 1550.0 )
	{
		x = (1550.0-track->get_d())/track->get_c();
		y = track->get_a()*x + track->get_b();
		z = track->get_c()*x + track->get_d();
	}
	else if( z < -1550.0 )
	{
		x = (-1550.0-track->get_d())/track->get_c();
		y = track->get_a()*x + track->get_b();
		z = track->get_c()*x + track->get_d();
	}
		
	line_3d.set_last(x * CLHEP::mm, y * CLHEP::mm, z * CLHEP::mm);
			
}

