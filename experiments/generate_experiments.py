import random
import fire
import random
import os


def generate(size, fault, seed, num_faulty=1, led_bins=0, length=300, lower=0, upper=1000):
	if num_faulty > size or num_faulty < 0:
		raise ValueError(f"`num_faulty` must be between 0 and {size}")
	show_leds = "true"
	if led_bins == 0:
		show_leds = "false"
		led_bins = 2 # need a default value that is not 0
		
	if size == 16 or size == 20:
		arena = f"""
            <!-- *********************** -->
    <!-- * Arena configuration * -->
    <!-- *********************** -->
    <arena size="3.5, 3.5, 2" center="0,0,1">

      <floor id="floor"
            source="loop_functions"
            pixels_per_meter="50" />

      <box id="wall_north" size="3,0.1,0.5" movable="false">
        <body position="0,1.5,0" orientation="0,0,0" />
      </box>
      <box id="wall_south" size="3,0.1,0.5" movable="false">
        <body position="0,-1.5,0" orientation="0,0,0" />
      </box>
      <box id="wall_east" size="0.1,3,0.5" movable="false">
        <body position="1.5,0,0" orientation="0,0,0" />
      </box>
      <box id="wall_west" size="0.1,3,0.5" movable="false">
        <body position="-1.5,0,0" orientation="0,0,0" />
      </box>

      <light id="light_1"
            position="-1.7,0,1.5"
            orientation="0,0,0"
            color="yellow"
            intensity="3.0"
            medium="leds" />

      <distribute>
        <position method="uniform" min="-1.5, -1.5,0" max="-1,1.5,0" />
        <orientation method="uniform" min="0,0,0" max="360,0,0" />
        <entity quantity="{size}" max_trials="5">
    <e-puck id="ep" rab_data_size="1000" rab_range="1">    <!-- using a high bandwidth of 100 bytes for now -->
            <controller config="efc" />
          </e-puck>
        </entity>
      </distribute>

    </arena>                                   
    """
		camera = f"""
        <camera>
          <placements>
            <placement index="0" position="1.99925,-0.0942354,11.1666" look_at="1.77948,-0.0942354,10.191" up="-0.975552,2.07473e-15,0.219767" lens_focal_length="65" />
          </placements>
        </camera>"""

	elif size == 64: 
		arena = f"""    
	<!-- *********************** -->
    <!-- * Arena configuration * -->
    <!-- *********************** -->
    <arena size="6, 6, 2" center="0,0,1">

      <floor id="floor"
            source="loop_functions"
            pixels_per_meter="50" />

      <box id="wall_north" size="6,0.1,0.5" movable="false">
        <body position="0,3,0" orientation="0,0,0" />
      </box>
      <box id="wall_south" size="6, 0.1,0.5" movable="false">
        <body position="0,-3,0" orientation="0,0,0" />
      </box>
      <box id="wall_east" size="0.1,6,0.5" movable="false">
        <body position="3,0,0" orientation="0,0,0" />
      </box>
      <box id="wall_west" size="0.1,6,0.5" movable="false">
        <body position="-3,0,0" orientation="0,0,0" />
      </box>

      <light id="light_1"
            position="-1.7,0,1.5"
            orientation="0,0,0"
            color="yellow"
            intensity="3.0"
            medium="leds" />

      <distribute>
        <position method="uniform" min="-3, -3,0" max="-2,3,0" />
        <orientation method="uniform" min="0,0,0" max="360,0,0" />
        <entity quantity="64" max_trials="5">
    <e-puck id="ep" rab_data_size="1000" rab_range="1">    <!-- using a high bandwidth of 100 bytes for now -->
            <controller config="efc" />
          </e-puck>
        </entity>
      </distribute>

    </arena>"""
		camera = f"""
        <camera>
          <placements>
            <placement index="0" position="9.92102,-0.00429149,14.0593" look_at="9.31433,-0.00429149,13.2644" up="-0.794941,-1.05405e-15,0.606687" lens_focal_length="65" />
          </placements>
        </camera>"""
	else:
		raise ValueError("Give valid size, 16 or 64")
	
	id_faulty = random.sample(range(size), num_faulty)
	faulty_times = random.choices(range(lower, upper), k=num_faulty)

	id_faulty_str = ' '.join(map(str, id_faulty))
	faulty_times_str = ' '.join(map(str, faulty_times))



	content = f"""<?xml version="1.0" ?>

  <!-- *************************************************** -->
  <!-- * A fully commented XML is diffusion_1.xml. Refer * -->
  <!-- * to it to have full information about what       * -->
  <!-- * these options mean.                             * -->
  <!-- *************************************************** -->

  <argos-configuration>

    <!-- ************************* -->
    <!-- * General configuration * -->
    <!-- ************************* -->
    <framework>
      <system threads="0" />
      <experiment length="{length}"
                  ticks_per_second="10"
                  random_seed="{seed}" />
    </framework>

    <!-- *************** -->
    <!-- * Controllers * -->
    <!-- *************** -->
    <controllers>

      <epuck_foraging_controller id="efc"
                                  library="build/controllers/epuck_foraging/libepuck_foraging.so">
        <actuators>
          <differential_steering implementation="default" noise_std_dev="0.1" />
          <leds implementation="default" medium="leds" />
          <range_and_bearing implementation="default" />
        </actuators>
        <sensors>
          <light implementation="default" show_rays="false" />
          <ground implementation="rot_z_only" />
          <proximity implementation="default" show_rays="false" noise_level="0.1" />
          <range_and_bearing implementation="medium" medium="rab" show_rays="false" noise_std_dev="0.01" />
          <differential_steering implementation="default" vel_noise_range="-0.1:0.1"  dist_noise_range="-1.0:1.0" />
        </sensors>
        <params>
          <experiment_run swarm_behavior="SWARM_FORAGING"
        fault_behavior="{fault}" 
                    id_faulty_robot="{id_faulty_str}"
                    injection_step="{faulty_times_str}"
                    show_leds="false" />
          <wheel_turning max_speed="5" />
          <state initial_rest_to_explore_prob="0.1"
          minimum_resting_time="50"
                minimum_unsuccessful_explore_time="6000000"
                minimum_search_for_place_in_nest_time="50" >
          </state>
        </params>
      </epuck_foraging_controller>

    </controllers>

    <!-- ****************** -->
    <!-- * Loop functions * -->
    <!-- ****************** -->
    <loop_functions library="build/loop_functions/foraging_loop_functions/libforaging_loop_functions.so"
                    label="foraging_loop_functions">
      <foraging items="1"
                radius="0.1"
          arenalength="3.0"
                output="nohup_{seed}.txt"
                concise_output="true"
                show_leds="{show_leds}"
				led_bins="{led_bins}" />
    </loop_functions>

	{arena}

    <!-- ******************* -->
    <!-- * Physics engines * -->
    <!-- ******************* -->
    <physics_engines>
      <dynamics2d id="dyn2d" iterations="50" />
    </physics_engines>

    <!-- ********* -->
    <!-- * Media * -->
    <!-- ********* -->
    <media>
      <range_and_bearing id="rab" />
      <led id="leds" />
    </media>

    <!-- ****************** -->
    <!-- * Visualization * -->
    <!-- ****************** -->
    <visualization>
      <qt-opengl>
		{camera}

      <user_functions library="build/loop_functions/id_loop_functions/libid_loop_functions"
                        label="id_qtuser_functions" /> 

        <user_functions label="foraging_qt_user_functions" />
	    <frame_grabbing directory="frames/exp{seed}"
                      format="png"
                      quality="100"
                      headless_grabbing="true"
                      headless_frame_size="1600x1200"
                      headless_frame_rate="1"/>
      </qt-opengl>
    </visualization>

  </argos-configuration>
  """
	if not os.path.exists("frames"):
		os.mkdir("frames")
	if not os.path.exists(f"frames/exp{seed}"):
		os.mkdir(f"frames/exp{seed}")
	with open(f"epuck_foraging_{seed}.argos", 'w') as out:
		out.write(content)



if __name__ == "__main__":
    fire.Fire(generate)