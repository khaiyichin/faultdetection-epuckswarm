#include "foraging_loop_functions.h"
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/utility/configuration/argos_configuration.h>
#include <argos3/plugins/robots/e-puck/simulator/epuck_entity.h>
#include <controllers/epuck_foraging/epuck_foraging.h>

/****************************************/
/****************************************/

CForagingLoopFunctions::CForagingLoopFunctions() :
    m_pcFloor(NULL),
    m_pcRNG(NULL),
    m_unCollectedFood(0)
{
}

/****************************************/
/****************************************/

void CForagingLoopFunctions::Init(TConfigurationNode& t_node)
{
    try
    {
        TConfigurationNode& tForaging = GetNode(t_node, "foraging");
        /* Get a pointer to the floor entity */
        m_pcFloor = &GetSpace().GetFloorEntity();
        /* Get the number of food items we want to be scattered from XML */
        UInt32 unFoodItems;
        GetNodeAttribute(tForaging, "items", unFoodItems);
        /* Get the number of food items we want to be scattered from XML */
        GetNodeAttribute(tForaging, "radius", m_fFoodSquareRadius);

        GetNodeAttribute(tForaging, "arenalength", fArenaLength);

        GetNodeAttribute(tForaging, "show_leds", m_bShowLeds);

        GetNodeAttribute(tForaging, "concise_output", m_bConciseData);

        GetNodeAttribute(tForaging, "led_bins", m_unLedBins);

        // Populate pre-defined colors based on the number of bins
        m_cLedColors = {CColor::YELLOW};

        if (m_unLedBins > 4 || m_unLedBins < 2)
        {
            THROW_ARGOSEXCEPTION("LED bins must be either 2, 3, or 4.");
        }
        else
        {
            if (m_unLedBins == 4)
            {
                // m_cLedColors.push_back(CColor(245, 166, 39, 255)); // orange
                m_cLedColors.push_back(CColor::BLUE);
            }

            if (m_unLedBins >= 3)
            {
                // m_cLedColors.push_back(CColor(165, 67, 0, 255)); // brown
                m_cLedColors.push_back(CColor::MAGENTA);
            }

            if (m_unLedBins >= 2)
            {
                m_cLedColors.push_back(CColor::RED);
            }
        }

        /*
       * if we dont want the foraging beacon to be close to the wall
       m_cForagingArenaSideX = CRange<Real>((1.0f / 4.0f) * fArenaLength, (fArenaLength / 3.0f));
       m_cForagingArenaSideY = CRange<Real>(-((fArenaLength / 4.0f) - 0.3f), (fArenaLength / 4.0f) - 0.3f);*/

#ifdef RESOURCE_POSITION_CHANGED
        m_fFoodSquareRadius = 0.1f;
        m_cForagingArenaSideX = CRange<Real>((1.0f / 3.0f) * fArenaLength, (fArenaLength / 2.0f) - 0.3f);
#else
        m_cForagingArenaSideX = CRange<Real>((1.0f / 3.0f) * fArenaLength, (fArenaLength / 2.0f) - 0.3f);
#endif

        m_cForagingArenaSideY = CRange<Real>(-((fArenaLength / 2.0f) - 0.3f), (fArenaLength / 2.0f) - 0.3f);

        m_fFoodSquareRadius *= (fArenaLength / 3.0f); // normalise resource patch to arena length
        m_fFoodSquareRadius *= m_fFoodSquareRadius;

        /* Create a new RNG */
        m_pcRNG = CRandom::CreateRNG("argos");
        /* Distribute uniformly the items in the environment */
        for(UInt32 i = 0; i < unFoodItems; ++i)
            m_cFoodPos.push_back(CVector2(m_pcRNG->Uniform(m_cForagingArenaSideX),
                                          m_pcRNG->Uniform(m_cForagingArenaSideY)));

        /* Get the output file name from XML */
        GetNodeAttribute(tForaging, "output", m_strOutput);
        /* Open the file, erasing its contents */
        m_cOutput.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
        //m_cOutput << "# clock\twalking\tresting\tcollected_food\tenergy" << std::endl;
    }
    catch(CARGoSException& ex)
            THROW_ARGOSEXCEPTION_NESTED("Error parsing foraging loop function.", ex);


    // counting the number of robots in the swarm
    u_num_epucks = 0u;
    CSpace::TMapPerType& m_cEpucks = GetSpace().GetEntitiesByType("e-puck");
    for(CSpace::TMapPerType::iterator it = m_cEpucks.begin(); it != m_cEpucks.end(); ++it)
    {
        u_num_epucks++;
    }

    for(CSpace::TMapPerType::iterator it = m_cEpucks.begin(); it != m_cEpucks.end(); ++it)
    {
        /* Get handle to e-puck entity and controller */
        CEPuckEntity& cEPuck = *any_cast<CEPuckEntity*>(it->second);
        CEPuckForaging& cController = dynamic_cast<CEPuckForaging&>(cEPuck.GetControllableEntity().GetController());
        cController.GetExperimentType().SetNumEPuckRobotsInSwarm(u_num_epucks);
    }
}

/****************************************/
/****************************************/

void CForagingLoopFunctions::Reset()
{
    /* Zero the counters */
    m_unCollectedFood = 0;
    /* Close the file */
    m_cOutput.close();
    /* Open the file, erasing its contents */
    m_cOutput.open(m_strOutput.c_str(), std::ios_base::trunc | std::ios_base::out);
    //m_cOutput << "# clock\tcollected_food" << std::endl;
    /* Distribute uniformly the items in the environment */
    for(UInt32 i = 0; i < m_cFoodPos.size(); ++i)
        m_cFoodPos[i].Set(m_pcRNG->Uniform(m_cForagingArenaSideX),
                          m_pcRNG->Uniform(m_cForagingArenaSideY));
}

/****************************************/
/****************************************/

void CForagingLoopFunctions::PreStep()
{
#ifdef RESOURCE_POSITION_CHANGED
    if(GetSpace().GetSimulationClock() == 10000.0)
    {
        m_fFoodSquareRadius = 0.1f;
        m_fFoodSquareRadius *= (fArenaLength / 3.0f); // normalise resource patch to arena length
        m_fFoodSquareRadius *= m_fFoodSquareRadius;

        m_cForagingArenaSideX = CRange<Real>((1.0f / 3.0f) * fArenaLength, (fArenaLength / 2.0f) - 0.3f);
        m_cForagingArenaSideY = CRange<Real>(-((fArenaLength / 2.0f) - 0.3f), (fArenaLength / 2.0f) - 0.3f);

        // remove the foraging site from the arena
        for(UInt32 i = 0; i < m_cFoodPos.size(); ++i)
            m_cFoodPos[i].Set(5.0f, 5.0f);

        m_pcFloor->SetChanged() ;
    }

    if(GetSpace().GetSimulationClock() == 20000.0)
    {
        m_fFoodSquareRadius = 0.2f;
        m_fFoodSquareRadius *= (fArenaLength / 3.0f); // normalise resource patch to arena length
        m_fFoodSquareRadius *= m_fFoodSquareRadius;

        m_cForagingArenaSideX = CRange<Real>(-0.9f, -0.5f); // nest at x-coordinate -1m to -1.5m
        m_cForagingArenaSideY = CRange<Real>(-((fArenaLength / 2.0f) - 0.3f), (fArenaLength / 2.0f) - 0.3f);

        /* Distribute uniformly the items in the environment */
        for(UInt32 i = 0; i < m_cFoodPos.size(); ++i)
            m_cFoodPos[i].Set(m_pcRNG->Uniform(m_cForagingArenaSideX), m_pcRNG->Uniform(m_cForagingArenaSideY));

        m_pcFloor->SetChanged() ;
    }
#endif


    m_unCollectedFood = 0;

    /* Check whether a robot is on a food item */
    CSpace::TMapPerType& m_cEPucks = GetSpace().GetEntitiesByType("e-puck");

    for(CSpace::TMapPerType::iterator it = m_cEPucks.begin(); it != m_cEPucks.end(); ++it)
    {
        /* Get handle to e-puck entity and controller */
        CEPuckEntity& cEPuck = *any_cast<CEPuckEntity*>(it->second);
        CEPuckForaging& cController = dynamic_cast<CEPuckForaging&>(cEPuck.GetControllableEntity().GetController());

        /* Get food data */
        CEPuckForaging::SFoodData& sFoodData = cController.GetFoodData();
        m_unCollectedFood += sFoodData.TotalFoodItems;

    }

    /* Output stuff to file */
    // m_cOutput << GetSpace().GetSimulationClock() << "\t"
    //           << m_unCollectedFood << std::endl;

    m_strCurrStepData = std::to_string(GetSpace().GetSimulationClock()) + "\t" + std::to_string(m_unCollectedFood) + "\n";

    /* Preparing a list of beacon robots to be then ignored by the fault detection algorithm*/
    for(CSpace::TMapPerType::iterator it = m_cEPucks.begin(); it != m_cEPucks.end(); ++it)
    {
        /* Get handle to e-puck entity and controller */
        CEPuckEntity& cEPuck = *any_cast<CEPuckEntity*>(it->second);
        CEPuckForaging& cController = dynamic_cast<CEPuckForaging&>(cEPuck.GetControllableEntity().GetController());
        cController.beaconrobots_ids.clear();

        for(CSpace::TMapPerType::iterator it1 = m_cEPucks.begin(); it1 != m_cEPucks.end(); ++it1)
        {
            CEPuckEntity& cEPuck_Observers = *any_cast<CEPuckEntity*>(it1->second);
            CEPuckForaging& cController1 = dynamic_cast<CEPuckForaging&>(cEPuck_Observers.GetControllableEntity().GetController());

            unsigned observers_rob_id = cController1.RobotIdStrToInt();

            if(cController1.GetStateData().State == CEPuckForaging::SStateData::STATE_BEACON)
            {
                cController.beaconrobots_ids.push_back(observers_rob_id);
            }
        }
    }
}

/****************************************/
/****************************************/

void CForagingLoopFunctions::Destroy()
{
    /* Close the file */
    m_cOutput.close();
}

/****************************************/
/****************************************/

CColor CForagingLoopFunctions::GetFloorColor(const CVector2& c_position_on_plane) // used to paint the floor by the floor entity
{
    if(c_position_on_plane.GetX() < -((1.0f / 3.0f) * fArenaLength))
        return CColor::GRAY50;

    for(UInt32 i = 0; i < m_cFoodPos.size(); ++i)
    {
        if((c_position_on_plane - m_cFoodPos[i]).SquareLength() < m_fFoodSquareRadius)
            return CColor::BLACK;
    }

    return CColor::WHITE;
}

/****************************************/
/****************************************/


void CForagingLoopFunctions::PostStep()
{
    if(GetSpace().GetSimulationClock() <= 450.0)
    {
        vec_strData.push_back(m_strCurrStepData);
        return;
    }

#ifndef RECORDSELFVOTESONLY
    CSpace::TMapPerType& m_cEpucks = GetSpace().GetEntitiesByType("e-puck");
    for(CSpace::TMapPerType::iterator it = m_cEpucks.begin(); it != m_cEpucks.end(); ++it)
    {
        /* Get handle to e-puck entity and controller */
        CEPuckEntity& cEPuck = *any_cast<CEPuckEntity*>(it->second);
        CEPuckForaging& cController = dynamic_cast<CEPuckForaging&>(cEPuck.GetControllableEntity().GetController());

        unsigned observed_rob_id = cController.RobotIdStrToInt();
        unsigned observed_rob_fv = cController.GetRobotFeatureVector();

        std::list<unsigned> list_Consensus_Tolerators, list_Consensus_Attackers;
        list_Consensus_Tolerators.clear(); list_Consensus_Attackers.clear();


        for(CSpace::TMapPerType::iterator it_ob = m_cEpucks.begin(); it_ob != m_cEpucks.end(); ++it_ob)
        {
            CEPuckEntity& cEPuck_Observers = *any_cast<CEPuckEntity*>(it_ob->second);
            CEPuckForaging& cController_Observers = dynamic_cast<CEPuckForaging&>(cEPuck_Observers.GetControllableEntity().GetController());

            unsigned observers_rob_id = cController_Observers.RobotIdStrToInt();

            t_listConsensusInfoOnRobotIds listConsensusInfoOnRobotIds = cController_Observers.GetListConsensusInfoOnRobotIds();

            for (t_listConsensusInfoOnRobotIds ::iterator it_con = listConsensusInfoOnRobotIds.begin(); it_con != listConsensusInfoOnRobotIds.end(); ++it_con)
            {
                if(it_con->uRobotId == observed_rob_id && it_con->consensus_state == 1)
                    list_Consensus_Attackers.push_back(observers_rob_id);

                else if(it_con->uRobotId == observed_rob_id && it_con->consensus_state == 2)
                    list_Consensus_Tolerators.push_back(observers_rob_id);
            }
        }

        // If concise data is desired, proceed only if data is meaningful
        if (m_bConciseData &&
            GetSpace().GetSimulationClock() % 100 <= 90) // hack because I don't know where they parameterize the 9+1 seconds polling time (Khai Yi 04/07/23)
        {
            break;
        }

        // m_cOutput << "Clock: " << GetSpace().GetSimulationClock() << "\t"
        //           << "Id: " << observed_rob_id << "\t"
        //           << "FV: " << observed_rob_fv << "\t";
        m_strCurrStepData += "Clock: " + std::to_string(GetSpace().GetSimulationClock()) + "\t"
                  + "Id: " + std::to_string(observed_rob_id) + "\t"
                  + "FV: " + std::to_string(observed_rob_fv) + "\t";

        // m_cOutput << "Consensus_Tolerators: ";
        m_strCurrStepData += "Consensus_Tolerators: ";
        for (std::list<unsigned>::iterator it_tolcon = list_Consensus_Tolerators.begin(); it_tolcon != list_Consensus_Tolerators.end(); ++it_tolcon)
            // m_cOutput << (*it_tolcon) << " ";
            m_strCurrStepData += std::to_string(*it_tolcon) + " ";
        for (int i = 0; i < u_num_epucks - list_Consensus_Tolerators.size(); ++i)
            // m_cOutput << " -1 ";
            m_strCurrStepData += " -1 ";

        // m_cOutput <<  "\t" << "Consensus_Attackers: ";
        m_strCurrStepData += "\tConsensus_Attackers: ";
        for (std::list<unsigned>::iterator it_atkcon = list_Consensus_Attackers.begin(); it_atkcon != list_Consensus_Attackers.end(); ++it_atkcon)
            // m_cOutput << (*it_atkcon) << " ";
            m_strCurrStepData += std::to_string(*it_atkcon) + " ";
        for (int i = 0; i < u_num_epucks - list_Consensus_Attackers.size(); ++i)
            // m_cOutput << " -1 ";
            m_strCurrStepData += " -1 ";
        // m_cOutput << std::endl;
        m_strCurrStepData += "\n";

        // Have robot display LEDs
        if (m_bShowLeds)
        {
            if((list_Consensus_Attackers.size() == list_Consensus_Tolerators.size()) && (list_Consensus_Tolerators.size() == 0))
                if (observed_rob_id == 15)
                    /*
                        Khai Yi 04/05/23
                        Turned this off to prevent the hardcoded robot to flash red; made it flash black instead so that there's no distinction
                    */
                    // cController.GetLEDsPtr()->SetAllColors(CColor::RED);
                    cController.GetLEDsPtr()->SetAllColors(CColor::BLACK);
                else
                    cController.GetLEDsPtr()->SetAllColors(CColor::BLACK);
            else if(list_Consensus_Attackers.size() > list_Consensus_Tolerators.size())
            {
                /*
                    Khai Yi 04/19/23
                    Set the robot's LED based on color codes (pre-defined based on the number of bins)
                */
                // cController.GetLEDsPtr()->SetAllColors(CColor::RED);
                int threshold = m_cEpucks.size() / m_unLedBins;

                // Decide the correct color to display
                for (int i = m_unLedBins - 1; i >= 0; --i)
                {
                    if (list_Consensus_Attackers.size() == 0) break; // shouldn't happen (since we're already in an `else if` that would disqualify this)
                    else if (list_Consensus_Attackers.size() > i * threshold)
                    {
                        cController.GetLEDsPtr()->SetAllColors(m_cLedColors[i]);
                        break;
                    }
                }
            }
            else
                cController.GetLEDsPtr()->SetAllColors(CColor::GREEN);
        }
    }
#else
    CSpace::TMapPerType& m_cEpucks = GetSpace().GetEntitiesByType("e-puck");
    for(CSpace::TMapPerType::iterator it = m_cEpucks.begin(); it != m_cEpucks.end(); ++it)
    {
        /* Get handle to e-puck entity and controller */
        CEPuckEntity& cEPuck = *any_cast<CEPuckEntity*>(it->second);
        CEPuckForaging& cController = dynamic_cast<CEPuckForaging&>(cEPuck.GetControllableEntity().GetController());

        unsigned observed_rob_id = cController.RobotIdStrToInt();
        unsigned observed_rob_fv = cController.GetRobotFeatureVector();

        std::list<unsigned> list_Voters_Tolerators, list_Voters_Attackers;
        list_Voters_Tolerators.clear(); list_Voters_Attackers.clear();


        for(CSpace::TMapPerType::iterator it_ob = m_cEpucks.begin(); it_ob != m_cEpucks.end(); ++it_ob)
        {
            CEPuckEntity& cEPuck_Observers = *any_cast<CEPuckEntity*>(it_ob->second);
            CEPuckForaging& cController_Observers = dynamic_cast<CEPuckForaging&>(cEPuck_Observers.GetControllableEntity().GetController());

            unsigned observers_rob_id = cController_Observers.RobotIdStrToInt();

            t_listVoteInformationRobots listVoteInformationRobots = cController_Observers.GetListVoteInfoOnRobotIds();

            for (t_listVoteInformationRobots::iterator it_vot = listVoteInformationRobots.begin(); it_vot != listVoteInformationRobots.end(); ++it_vot)
            {
                assert(it_vot->uVoterIds.size() == 1u); // only self-vote should be registered
                assert(it_vot->attackvote_count + it_vot->toleratevote_count == 1u); // only self-vote should be registered

                if(it_vot->uRobotId == observed_rob_id && it_vot->attackvote_count > 0)
                    list_Voters_Attackers.push_back(observers_rob_id);

                else if(it_vot->uRobotId == observed_rob_id && it_vot->toleratevote_count > 0)
                    list_Voters_Tolerators.push_back(observers_rob_id);
            }
        }

        m_cOutput << "Clock: " << GetSpace().GetSimulationClock() << "\t"
                  << "Id: " << observed_rob_id << "\t"
                  << "FV: " << observed_rob_fv << "\t";

        m_cOutput << "Voters_Tolerators: ";
        for (std::list<unsigned>::iterator it_tolcon = list_Voters_Tolerators.begin(); it_tolcon != list_Voters_Tolerators.end(); ++it_tolcon)
            m_cOutput << (*it_tolcon) << " ";
        for (int i = 0; i < u_num_epucks - list_Voters_Tolerators.size(); ++i)
            m_cOutput << " -1 ";

        m_cOutput <<  "\t" << "Voters_Attackers: ";
        for (std::list<unsigned>::iterator it_atkcon = list_Voters_Attackers.begin(); it_atkcon != list_Voters_Attackers.end(); ++it_atkcon)
            m_cOutput << (*it_atkcon) << " ";
        for (int i = 0; i < u_num_epucks - list_Voters_Attackers.size(); ++i)
            m_cOutput << " -1 ";
        m_cOutput << std::endl;
    }
#endif

    vec_strData.push_back(m_strCurrStepData);
}

/****************************************/
/****************************************/

void CForagingLoopFunctions::PostExperiment()
{
    /* Writing the ids of the beacon robots */

    bool beaconspresent(false);
    std::string post_exp_str = "";

    // m_cOutput << "BeaconRobotIds: " << "\t";
    post_exp_str += "BeaconRobotIds: \t";

    CSpace::TMapPerType& m_cEpucks = GetSpace().GetEntitiesByType("e-puck");
    for(CSpace::TMapPerType::iterator it = m_cEpucks.begin(); it != m_cEpucks.end(); ++it)
    {
        /* Get handle to e-puck entity and controller */
        CEPuckEntity& cEPuck = *any_cast<CEPuckEntity*>(it->second);
        CEPuckForaging& cController = dynamic_cast<CEPuckForaging&>(cEPuck.GetControllableEntity().GetController());

        if(cController.GetStateData().State == CEPuckForaging::SStateData::STATE_BEACON)
        {
            beaconspresent = true;
            // m_cOutput <<  cController.RobotIdStrToInt() << " ";
            post_exp_str += std::to_string(cController.RobotIdStrToInt()) + " ";
        }
    }

    if(!beaconspresent)
        // m_cOutput << "-1 ";
        post_exp_str += "-1 ";

    // m_cOutput << std::endl << std::endl;
    post_exp_str += "\n\n";

    /* Writing the amount of time each robot spent in each state */
    for(CSpace::TMapPerType::iterator it = m_cEpucks.begin(); it != m_cEpucks.end(); ++it)
    {
        /* Get handle to e-puck entity and controller */
        CEPuckEntity& cEPuck = *any_cast<CEPuckEntity*>(it->second);
        CEPuckForaging& cController = dynamic_cast<CEPuckForaging&>(cEPuck.GetControllableEntity().GetController());

        // m_cOutput << "TimeForagingStates: " << cController.RobotIdStrToInt() << " " << cController.TIME_STATE_RESTING1 <<  " " << cController.TIME_STATE_EXPLORING1 <<  " " << cController.TIME_STATE_BEACON1 <<  " " << cController.TIME_STATE_RESTING_AT_FOOD1 <<   " " << cController.TIME_STATE_RETURN_TO_NEST1 << std::endl;
        post_exp_str += "TimeForagingStates: " + std::to_string(cController.RobotIdStrToInt()) + " " + std::to_string(cController.TIME_STATE_RESTING1) +  " " + std::to_string(cController.TIME_STATE_EXPLORING1) +  " " + std::to_string(cController.TIME_STATE_BEACON1) +  " " + std::to_string(cController.TIME_STATE_RESTING_AT_FOOD1) +   " " + std::to_string(cController.TIME_STATE_RETURN_TO_NEST1) + "\n";
    }

    // m_cOutput << std::endl ;
    post_exp_str += "\n";

    /* Writing the amount of time each robot spent in each state */
    for(CSpace::TMapPerType::iterator it = m_cEpucks.begin(); it != m_cEpucks.end(); ++it)
    {
        /* Get handle to e-puck entity and controller */
        CEPuckEntity& cEPuck = *any_cast<CEPuckEntity*>(it->second);
        CEPuckForaging& cController = dynamic_cast<CEPuckForaging&>(cEPuck.GetControllableEntity().GetController());

        // m_cOutput << "TimeForagingStates: " << cController.RobotIdStrToInt() << " " << cController.TIME_STATE_RESTING2 <<  " " << cController.TIME_STATE_EXPLORING2 <<  " " << cController.TIME_STATE_BEACON2 <<  " " << cController.TIME_STATE_RESTING_AT_FOOD2 <<   " " << cController.TIME_STATE_RETURN_TO_NEST2 << std::endl;
        post_exp_str += "TimeForagingStates: " + std::to_string(cController.RobotIdStrToInt()) + " " + std::to_string(cController.TIME_STATE_RESTING2) +  " " + std::to_string(cController.TIME_STATE_EXPLORING2) +  " " + std::to_string(cController.TIME_STATE_BEACON2) +  " " + std::to_string(cController.TIME_STATE_RESTING_AT_FOOD2) +   " " + std::to_string(cController.TIME_STATE_RETURN_TO_NEST2) + "\n";
    }

    // m_cOutput << std::endl ;
    post_exp_str += "\n" ;

    /* Writing the amount of time each robot spent in each state */
    for(CSpace::TMapPerType::iterator it = m_cEpucks.begin(); it != m_cEpucks.end(); ++it)
    {
        /* Get handle to e-puck entity and controller */
        CEPuckEntity& cEPuck = *any_cast<CEPuckEntity*>(it->second);
        CEPuckForaging& cController = dynamic_cast<CEPuckForaging&>(cEPuck.GetControllableEntity().GetController());

        // m_cOutput << "TimeForagingStates: " << cController.RobotIdStrToInt() << " " << cController.TIME_STATE_RESTING <<  " " << cController.TIME_STATE_EXPLORING <<  " " << cController.TIME_STATE_BEACON <<  " " << cController.TIME_STATE_RESTING_AT_FOOD <<   " " << cController.TIME_STATE_RETURN_TO_NEST << std::endl;
        post_exp_str += "TimeForagingStates: " + std::to_string(cController.RobotIdStrToInt()) + " " + std::to_string(cController.TIME_STATE_RESTING) +  " " + std::to_string(cController.TIME_STATE_EXPLORING) +  " " + std::to_string(cController.TIME_STATE_BEACON) +  " " + std::to_string(cController.TIME_STATE_RESTING_AT_FOOD) +   " " + std::to_string(cController.TIME_STATE_RETURN_TO_NEST) + "\n";
    }
    vec_strData.push_back(post_exp_str);

    // Write to file
    for (auto itr = vec_strData.begin(); itr != vec_strData.end(); ++itr)
    {
        m_cOutput << *itr;
    }
}

/****************************************/
/****************************************/

REGISTER_LOOP_FUNCTIONS(CForagingLoopFunctions, "foraging_loop_functions")
