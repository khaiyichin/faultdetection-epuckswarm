/*
 * AUTHOR: Danesh Tarapore <daneshtarapore@gmail.com>
 *
 * An example foraging controller for the foot-bot.
 *
 * This controller is meant to be used with the XML file:
 *    experiments/foraging.argos
 */

//! TODO: Use the num of observations to select from different relayed FVs Bayesian Inferred FVs
//! TODO: Refresh the priors perodically


#ifndef EPUCK_HOMSWARM_H
#define EPUCK_HOMSWARM_H

/*
 * Include some necessary headers.
 */

#include <iostream>
#include <vector>
#include <algorithm>    // std::sort

/****************************************/
/****************************************/
/* ARGoS headers */

/* Definition of the CCI_Controller class. */
#include <argos3/core/control_interface/ci_controller.h>

/* Definition of the differential steering actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>

/* Definition of proximity sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_proximity_sensor.h>

/* Definition of the differential steering wheel encoder */
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_sensor.h>

/* Definition of the LEDs actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>

/* Definition of the range and bearing actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_actuator.h>

/* Definition of the range and bearing sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>

/* Definitions for random number generation */
#include <argos3/core/utility/math/rng.h>

/****************************************/
/****************************************/
/* Definitions for behaviors used to control robot */

#include "behavior.h"
#include "aggregatebehavior.h"
#include "dispersebehavior.h"
#include "randomwalkbehavior.h"
#include "phototaxisbehavior.h"
#include "antiphototaxisbehavior.h"
#include "homingtofoodbeaconbehavior.h"
#include "circlebehavior.h"
#include "flockingbehavior.h"

/****************************************/
/****************************************/
/* Definition of functions to estimate feature-vectors - proprioceptively, or by observation */

#include "propriofeaturevector.h"
#include "observedfeaturevector.h"
#include "bayesianinferencefeaturevector.h"

/****************************************/
/****************************************/
/* Definition of functions to assimilate the different feature-vectors and perform abnormality detection */

#include "sensingandcommunication.h"
#include "featurevectorsinrobotagent.h"
#include "crminrobotagent_optimised.h"

/****************************************/
/****************************************/

#ifdef DISABLE_SWARMCOALITION
//! FOR PHASE C --  Disabled swarm coalition formation on detected abnormal behavior
#define RECORDSELFVOTESONLY
#endif

/****************************************/
/****************************************/

/*
 * All the ARGoS stuff in the 'argos' namespace.
 * With this statement, you save typing argos:: every time.
 */
using namespace argos;

/*
 * A controller is simply an implementation of the CCI_Controller class.
 */
class CEPuckHomSwarm : public CCI_Controller
{

public:

    struct ExperimentToRun
    {
        /* The type of experiment to run */
        enum SwarmBehavior
        {
            SWARM_AGGREGATION = 0,
            SWARM_DISPERSION,
            SWARM_HOMING,
            SWARM_FLOCKING,
            SWARM_STOP,
            SWARM_NONE
        };
        enum SwarmBehavior SBehavior;


        enum SwarmBehavior SBehavior_Trans, SBehavior_Current; Real time_between_robots_trans_behav;
        std::list <unsigned> robot_ids_behav1; std::list <unsigned> robot_ids_behav2;
        unsigned u_num_epucks; // number of epucks in swarm

        /* The possible faults on robot */
        enum FaultBehavior
        {
            FAULT_NONE = 0,

            /*faults whose effects cause one of the following four general failures */
            FAULT_STRAIGHTLINE,
            FAULT_RANDOMWALK,
            FAULT_CIRCLE,
            FAULT_STOP,

            /* Implementing the faults themselves. The resulting behaviors will now depend on the normal behavior implementation. */
            FAULT_PROXIMITYSENSORS_SETMIN,
            FAULT_PROXIMITYSENSORS_SETMAX,
            FAULT_PROXIMITYSENSORS_SETRANDOM,
            FAULT_PROXIMITYSENSORS_SETOFFSET,

            FAULT_RABSENSOR_SETOFFSET,
            FAULT_RABSENSOR_MISSINGRECEIVERS,

            FAULT_ACTUATOR_LWHEEL_SETZERO,
            FAULT_ACTUATOR_RWHEEL_SETZERO,
            FAULT_ACTUATOR_BWHEELS_SETZERO,

            FAULT_SOFTWARE,

            FAULT_POWER_FAILURE
        } FBehavior;

        std::string id_FaultyRobotInSwarm;
        std::string swarmbehav;


        ExperimentToRun();
        void Init(TConfigurationNode& t_node);

        inline void SetNumEPuckRobotsInSwarm(unsigned num_epucks)
        {
            u_num_epucks = num_epucks;
        }


    };

    /*
    * The following variables are used as parameters for
    * turning during navigation. You can set their value
    * in the <parameters> section of the XML configuration
    * file, under the
    * <controllers><footbot_foraging_controller><parameters><wheel_turning>
    * section.
    */
    struct SWheelTurningParams
    {
        Real MaxSpeed;
        void Init(TConfigurationNode& t_tree);
    };


    struct RobotDetails
    {
        Real iterations_per_second; /* controlcycles run per second*/
        Real seconds_per_iterations;
        Real HALF_INTERWHEEL_DISTANCE;  // in m
        Real INTERWHEEL_DISTANCE;  // in m
        Real WHEEL_RADIUS;  // in m

        CRadians m_cNoTurnOnAngleThreshold; CRadians m_cSoftTurnOnAngleThreshold;

        Real MaxLinearSpeed; //cm/ (controlcycle)
        Real MaxLinearAcceleration; //cm/controlcycle/controlcycle

        Real MaxAngularSpeed; //rad/controlcycle
        Real MaxAngularAcceleration; //rad/controlcycle/controlcycle;

        RobotDetails()
        {
            iterations_per_second  = 10.0f; /*10 ticks per second so dt=0.01s. i.e., the controlcycle is run 10 times per second*/
            seconds_per_iterations = 1.0f / iterations_per_second;
            HALF_INTERWHEEL_DISTANCE = 0.053f * 0.5f;  // m
            INTERWHEEL_DISTANCE  = 0.053f;  // m
            WHEEL_RADIUS = 0.0205f;  // m

            m_cNoTurnOnAngleThreshold   = ToRadians(CDegrees(10.0f)); //10.0 - straight to food spot; 35.0 spiral to food spot
            m_cSoftTurnOnAngleThreshold = ToRadians(CDegrees(70.0f));
        }

        void SetKinematicDetails(Real f_MaxLeftWheelSpeed, Real f_MaxRightWheelSpeed) // arguments are speeds in cm/s
        {
            //std::cout << " f_MaxLeftWheelSpeed " << f_MaxLeftWheelSpeed << std::endl;

            // the max linear speed is 1 cm/controlcycle.
            // so, MaxLinearSpeed = 1
            MaxLinearSpeed        = ((f_MaxLeftWheelSpeed + f_MaxRightWheelSpeed) / 2.0f) * seconds_per_iterations;  //in cm/ (controlcycle)

            //std::cout << " MaxLinearSpeed " << MaxLinearSpeed << std::endl;

            // as the max speed is 1 cm/controlcycle (resultant speed is always positive as the robot does not traverse backwards), the max acceleration is +/-1 cm/controlcycle/controlcycle
            // so, MaxLinearAcceleration = |+/-1 cm/controlcycle/controlcycle| = 1cm/controlcycle/controlcycle
            MaxLinearAcceleration = MaxLinearSpeed;

            // the max angular speed is +/-21.6 degrees/controlcycle,
            // so MaxAngularSpeed = |+/-21.621 degrees/controlcycle| = |21.621 degrees/controlcycle|
            MaxAngularSpeed       = ((f_MaxLeftWheelSpeed + f_MaxRightWheelSpeed) /
                                     (INTERWHEEL_DISTANCE * 100.0f)) * seconds_per_iterations; //rad/controlcycle

            // as the max angular speed is +/-21.6 degrees/controlcycle, the max acceleration is +/-43.2421 radians/controlcycle/controlcycle
            // so MaxAngularAcceleration = |+/-43.2421 degrees/controlcycle/controlcycle| = 43.2421 degrees/controlcycle/controlcycle
            MaxAngularAcceleration   = 2.0f * MaxAngularSpeed; //rad/controlcycle/controlcycle;
        }
    };

    RobotDetails m_sRobotDetails;

public:

    /* Class constructor. */
    CEPuckHomSwarm();
    /* Class destructor. */
    virtual ~CEPuckHomSwarm();

    /*
     *
     *
    */
    virtual void CopyRobotDetails(RobotDetails& sRobotDetails);

    /*
    * This function initializes the controller.
    * The 't_node' variable points to the <parameters> section in the XML
    * file in the <controllers><footbot_foraging_controller> section.
    */
    virtual void Init(TConfigurationNode& t_node);

    /*
     *
     *
     */
    unsigned SumFVDist(t_listFVsSensed& FVsSensed);

    /*
    * This function is called once every time step.
    * The length of the time step is set in the XML file.
    */
    virtual void ControlStep();


    CCI_LEDsActuator* GetLEDsPtr() {return m_pcLEDs;}


    /*
     * Execute general faults
    */
    virtual void RunGeneralFaults();

    /*
     * Run one of the swarm experiments (Aggregation, Dispersion, Homing)
    */
    virtual void RunHomogeneousSwarmExperiment();

    /*
    * This function resets the controller to its state right after the
    * Init().
    * It is called when you press the reset button in the GUI.
    */
    virtual void Reset();

    /*
     *  This function returns the interger cast of the string robot id
    */
    virtual unsigned RobotIdStrToInt();

    /*
    * Called to cleanup what done by Init() when the experiment finishes.
    * In this example controller there is no need for clean anything up,
    * so the function could have been omitted. It's here just for
    * completeness.
    */
    virtual void Destroy() {}

    virtual t_listConsensusInfoOnRobotIds& GetListConsensusInfoOnRobotIds()
    {
        return listConsensusInfoOnRobotIds;
    }

    virtual t_listVoteInformationRobots& GetListVoteInfoOnRobotIds()
    {
        return listVoteInformationRobots;
    }

    /*
     * For debugging purposes
     */
    virtual unsigned GetRobotFeatureVector()
    {
        return m_uRobotFV;
    }

    /*
    * Returns the experiment type
    */
    inline ExperimentToRun& GetExperimentType()
    {
        return m_sExpRun;
    }

    virtual CObservedFeatureVector& GetObservedFeatureVectors()
    {
        return m_cObservationFeatureVector;
    }


    t_listFVsSensed&             GetListFVsSensed()         {return listFVsSensed;}
    t_listMapFVsToRobotIds&      GetMapFVsSensed()          {return listMapFVsToRobotIds;}

    Real m_fInternalRobotTimer; Real m_fRobotTimerAtStart;
    Real m_fCRM_RUN_TIMESTAMP;
    bool b_CRM_Run;

    std::vector <int> beaconrobots_ids;

    unsigned m_uEPuckRABDataIndex;

    //static UInt8 BEACON_SIGNAL, NEST_BEACON_SIGNAL;

private:

    std::vector<Real> GetIRSensorReadings(bool b_DamagedRobot, ExperimentToRun::FaultBehavior fault_type)
    {
        std::vector<Real> sensor_readings = m_pcProximity->GetReadings();

        if(!b_DamagedRobot)
            return sensor_readings;

        if(fault_type == ExperimentToRun::FaultBehavior::FAULT_PROXIMITYSENSORS_SETMIN)
        {
            /* Front four IR sensors */
            sensor_readings[0] = 0.0f; sensor_readings[1] = 0.0f; sensor_readings[7] = 0.0f; sensor_readings[6] = 0.0f;

            //sensor_readings[2].Value = 0.0f; sensor_readings[3].Value = 0.0f;
            //sensor_readings[4].Value = 0.0f; sensor_readings[5].Value = 0.0f;

            return sensor_readings;
        }
        else if(fault_type == ExperimentToRun::FaultBehavior::FAULT_PROXIMITYSENSORS_SETMAX)
        {
            /* Front four IR sensors */
            sensor_readings[0] = 1.0f; sensor_readings[1] = 1.0f; sensor_readings[7] = 1.0f; sensor_readings[6] = 1.0f;

            //sensor_readings[2].Value = 1.0f; sensor_readings[3].Value = 1.0f;
            return sensor_readings;
        }
        else if(fault_type == ExperimentToRun::FaultBehavior::FAULT_PROXIMITYSENSORS_SETRANDOM)
        {
            /* Front four IR sensors */
            sensor_readings[0] = m_pcRNG->Uniform(CRange<Real>(0.0f, 1.0f));
            sensor_readings[1] = m_pcRNG->Uniform(CRange<Real>(0.0f, 1.0f));
            sensor_readings[7] = m_pcRNG->Uniform(CRange<Real>(0.0f, 1.0f));
            sensor_readings[6] = m_pcRNG->Uniform(CRange<Real>(0.0f, 1.0f));

            return sensor_readings;
        }
        else if(fault_type == ExperimentToRun::FaultBehavior::FAULT_PROXIMITYSENSORS_SETOFFSET)
        {
            /* Front four IR sensors */
            sensor_readings[0] += m_pcRNG->Uniform(CRange<Real>(-0.5f, 0.5f));
            sensor_readings[1] += m_pcRNG->Uniform(CRange<Real>(-0.5f, 0.5f));
            sensor_readings[7] += m_pcRNG->Uniform(CRange<Real>(-0.5f, 0.5f));
            sensor_readings[6] += m_pcRNG->Uniform(CRange<Real>(-0.5f, 0.5f));


            if(sensor_readings[0] > 1.0f)
                sensor_readings[0] = 1.0f;
            if(sensor_readings[0] < 0.0f)
                sensor_readings[0] = 0.0f;

            if(sensor_readings[1] > 1.0f)
                sensor_readings[1] = 1.0f;
            if(sensor_readings[1] < 0.0f)
                sensor_readings[1] = 0.0f;

            if(sensor_readings[7] > 1.0f)
                sensor_readings[7] = 1.0f;
            if(sensor_readings[7] < 0.0f)
                sensor_readings[7] = 0.0f;

            if(sensor_readings[6] > 1.0f)
                sensor_readings[6] = 1.0f;
            if(sensor_readings[6] < 0.0f)
                sensor_readings[6] = 0.0f;

            return sensor_readings;
        }

        else
        {
            /* the robot is running one of the general faults or one of the specific faults that doesnot influence IR sensor readings*/
            return sensor_readings;
        }
    }


    CCI_RangeAndBearingSensor::TReadings GetRABSensorReadings(bool b_DamagedRobot, ExperimentToRun::FaultBehavior fault_type)
    {
        CCI_RangeAndBearingSensor::TReadings sensor_readings = m_pcRABS->GetReadings();

        if(!b_DamagedRobot)
            return sensor_readings;

        if(fault_type == ExperimentToRun::FaultBehavior::FAULT_RABSENSOR_SETOFFSET)
        {
            for(size_t i = 0; i <  sensor_readings.size(); ++i)
            {
                CVector2 tmp(sensor_readings[i].Range, sensor_readings[i].HorizontalBearing);
                tmp += CVector2(m_pcRNG->Uniform(CRange<Real>(75.0f, 100.0f)),
                                m_pcRNG->Uniform(CRange<CRadians>(-CRadians::PI, CRadians::PI)));

                sensor_readings[i].Range             = tmp.Length();
                sensor_readings[i].HorizontalBearing = tmp.Angle();
            }

            return sensor_readings;
        }

        else if(fault_type == ExperimentToRun::FAULT_RABSENSOR_MISSINGRECEIVERS)
        {
            Real ReceiverSensingInterval[12][2];

            ReceiverSensingInterval[0][0] = 0.0f;                    ReceiverSensingInterval[0][1] = 15.0f + (50.0f - 15.0f)/2.0f;
            ReceiverSensingInterval[1][0] = 15.0f + (50.0f - 15.0f)/2.0f;    ReceiverSensingInterval[1][1] = 50.0f + (75.0f - 50.0f)/2.0f;
            ReceiverSensingInterval[2][0] = 50.0f + (75.0f - 50.0f)/2.0f;    ReceiverSensingInterval[2][1] = 75.0f + (105.0f - 75.0f)/2.0f;
            ReceiverSensingInterval[3][0] = 75.0f + (105.0f - 75.0f)/2.0f;   ReceiverSensingInterval[3][1] = 105.0f + (133.0f - 105.0f)/2.0f;
            ReceiverSensingInterval[4][0] = 105.0f + (133.0f - 105.0f)/2.0f;  ReceiverSensingInterval[4][1] = 133.0f + (159.0f - 133.0f)/2.0f;
            ReceiverSensingInterval[5][0] = 133.0f + (159.0f - 133.0f)/2.0f;  ReceiverSensingInterval[5][1] = 159.0f + (195.0f - 159.0f)/2.0f;
            ReceiverSensingInterval[6][0] = 159.0f + (195.0f - 159.0f)/2.0f;  ReceiverSensingInterval[6][1] = 195.0f + (225.0f - 195.0f)/2.0f;
            ReceiverSensingInterval[7][0] = 195.0f + (225.0f - 195.0f)/2.0f;  ReceiverSensingInterval[7][1] = 225.0f + (255.0f - 225.0f)/2.0f;
            ReceiverSensingInterval[8][0] = 225.0f + (255.0f - 225.0f)/2.0f;  ReceiverSensingInterval[8][1] = 255.0f + (283.0f - 255.0f)/2.0f;
            ReceiverSensingInterval[9][0] = 255.0f + (283.0f - 255.0f)/2.0f;  ReceiverSensingInterval[9][1] = 283.0f + (310.0f - 283.0f)/2.0f;
            ReceiverSensingInterval[10][0] = 283.0f + (310.0f - 283.0f)/2.0f;  ReceiverSensingInterval[10][1] = 310.0f + (345.0f - 310.0f)/2.0f;
            ReceiverSensingInterval[11][0] = 310.0f + (345.0f - 310.0f)/2.0f;  ReceiverSensingInterval[11][1] = 360.0f;

            Real startangle, endangle;
            // assume the front two receivers numbered 0 and 11 are missing
            for(size_t i = 0; i <  sensor_readings.size(); ++i)
            {
                CRadians BearingAngle     = sensor_readings[i].HorizontalBearing;
                Real BearingAngle_Degrees = ToDegrees(BearingAngle).UnsignedNormalize().GetValue();

                assert(BearingAngle_Degrees >= 0.0f);

                startangle = 105.0f + (133.0f - 105.0f) / 2.0f;
                endangle   = 225.0f + (255.0f - 225.0f) / 2.0f;
                if((BearingAngle_Degrees >= 0 && BearingAngle_Degrees <= startangle) || (BearingAngle_Degrees >= endangle && BearingAngle_Degrees <= 360.0f))
                {
                    if(BearingAngle_Degrees >= 0 && BearingAngle_Degrees <= startangle)
                        BearingAngle_Degrees = startangle;
                    else
                        BearingAngle_Degrees = endangle;

                    //sensor_readings[i].Range             = ;
                    sensor_readings[i].HorizontalBearing = ToRadians(CDegrees(BearingAngle_Degrees));

                    continue;
                }
            }
            return sensor_readings;
        }

        else
        {
            /* the robot is running one of the general faults or one of the specific faults that doesnot influence IR sensor readings*/
            return sensor_readings;
        }
    }

private:

    TBehaviorVector             m_vecBehaviors;
    bool                        b_damagedrobot;     // true if robot is damaged

    CFlockingBehavior*          m_pFlockingBehavior;

    unsigned                    u_num_consequtivecollisions;

    CProprioceptiveFeatureVector       m_cProprioceptiveFeatureVector;
    CObservedFeatureVector             m_cObservationFeatureVector;
    CBayesianInferenceFeatureVector    m_cBayesianInferredFeatureVector;

    t_listFVsSensed               listFVsSensed;
    t_listMapFVsToRobotIds        listMapFVsToRobotIds; // ids and fvs of observed neighbours, including ids and fvs the neighbours have relayed to you
    t_listMapFVsToRobotIds        listMapFVsToRobotIds_relay; // ids and fvs of observed neighbours - for you to relay to your neighbours.

    t_listConsensusInfoOnRobotIds listConsensusInfoOnRobotIds; // consensus established on the following robots

    t_listVoteInformationRobots   listVoteInformationRobots;   // list of registered votes for each observed robot id, and the corresponding voter ids

    CRMinRobotAgentOptimised*     crminAgent;

    /* Pointer to the differential steering actuator */
    CCI_DifferentialSteeringActuator* m_pcWheels;

    /* Pointer to the differential steering encoder */
    CCI_DifferentialSteeringSensor* m_pcWheelsEncoder;
    /* Pointer to the LEDs actuator */
    CCI_LEDsActuator* m_pcLEDs;
    /* Pointer to the range and bearing actuator */
    CCI_RangeAndBearingActuator*  m_pcRABA;
    /* Pointer to the range and bearing sensor */
    CCI_RangeAndBearingSensor* m_pcRABS;
    /* Pointer to the foot-bot proximity sensor */
    CCI_ProximitySensor* m_pcProximity;

    /* The random number generator */
    CRandom::CRNG* m_pcRNG;
    CRandom::CRNG* m_pcRNG_FVs; // we have a separate RNG for forgetting FVs. This way the actual behaviors based on random walk are independent from the running of the CRM.


    /*Information on the experiment to run - the swarm behavior and the faulty behavior*/
    ExperimentToRun m_sExpRun;
    /* The turning parameters */
    SWheelTurningParams m_sWheelTurningParams;

    unsigned m_uRobotId, m_uRobotFV;

};

#endif
