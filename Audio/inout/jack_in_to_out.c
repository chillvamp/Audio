/**
 * A simple 1-input to 1-output JACK client.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

jack_port_t *input_port1;
jack_port_t *input_port2;
jack_port_t *output_port;
jack_default_audio_sample_t buffer_4delay[5*48000];
jack_default_audio_sample_t buff[200][1024];


jack_client_t *client;
int i,j,delay_nframes,delay_pseudo;
float delay_secs;

/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by 
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */
int jack_callback (jack_nframes_t nframes, void *arg){
	jack_default_audio_sample_t **in, *out;
	in[0]= jack_port_get_buffer (input_port1, nframes);
	in[1]= jack_port_get_buffer (input_port2, nframes);
	out = jack_port_get_buffer (output_port, nframes);

	//printf  ("%i  \n" ,(int)(delay_nframes/nframes)    );


	
	
	
	for (i=0;i<nframes;++i)
		{

		
		if (delay_nframes<=1024){	
			if (i<delay_nframes)	out [i]= buffer_4delay[nframes- delay_nframes + i];
			else out[i]= in[0][i- delay_nframes];
			buffer_4delay[i]=in[0][i];
									}
		else{









			delay_pseudo=delay_nframes%nframes;
			int delay_bloques= (int)(delay_nframes/nframes);	
			int indj=   j%(delay_nframes/nframes);
			//printf  (" indj %i,j  %i, delaybloques,%i \n" ,indj , j  , delay_bloques);
			out[i]=buff[indj][i];
			buff[indj][i]= in[0][i];

			

			
				

			
			++j;


			if (j>56789) j=0;

			}
		
		

		//out[i]=buffer_4delay[(int)(delay_nframes/nframes)][i];
		
		}
	
	//memcpy (out, in, nframes * sizeof (jack_default_audio_sample_t));
	return 0;
}


/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg){
	exit (1);
}


int main (int argc, char *argv[]) {
	const char *client_name = "in_to_out";
	jack_options_t options = JackNoStartServer;
	jack_status_t status;
	j=0;

	
    printf("Number of SECONDSs to be delayed (max=5):");
    scanf("%f", &delay_secs);
    if (delay_secs>=4.9) delay_secs=4.9;
    delay_nframes= (int)(delay_secs* 48000);

    //scanf("%d", &delay_nframes);
	
	/* open a client connection to the JACK server */
	client = jack_client_open (client_name, options, &status);
	if (client == NULL){
		/* if connection failed, say why */
		printf ("jack_client_open() failed, status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			printf ("Unable to connect to JACK server.\n");
		}
		exit (1);
	}
	
	/* if connection was successful, check if the name we proposed is not in use */
	if (status & JackNameNotUnique){
		client_name = jack_get_client_name(client);
		printf ("Warning: other agent with our name is running, `%s' has been assigned to us.\n", client_name);
	}
	
	/* tell the JACK server to call 'jack_callback()' whenever there is work to be done. */
	jack_set_process_callback (client, jack_callback, 0);
	
	
	/* tell the JACK server to call 'jack_shutdown()' if it ever shuts down,
	   either entirely, or if it just decides to stop calling us. */
	jack_on_shutdown (client, jack_shutdown, 0);
	
	
	/* display the current sample rate. */
	printf ("Engine sample rate: %d\n", jack_get_sample_rate (client));
	
	
	/* create the agent input port */
	input_port1 = jack_port_register (client, "input1", JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput, 0);
	input_port2 = jack_port_register (client, "input2", JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput, 0);
	/* create the agent output port */
	output_port = jack_port_register (client, "output",JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput, 0);
	
	/* check that both ports were created succesfully */
	if ((input_port1 == NULL) || (output_port == NULL)) {
		printf("Could not create agent ports. Have we reached the maximum amount of JACK agent ports?\n");
		exit (1);
	}
	
	
	/* Tell the JACK server that we are ready to roll.
	   Our jack_callback() callback will start running now. */
	if (jack_activate (client)) {
		printf ("Cannot activate client.");
		exit (1);
	}
	
	printf ("Agent activated.\n");
	
	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */
	printf ("Connecting ports... ");
	 
	/* Assign our input port to a server output port*/
	// Find possible output server port names
	const char **serverports_names;
	serverports_names = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
	if (serverports_names == NULL) {
		printf("No available physical capture (server output) ports.\n");
		exit (1);
	}
	// Connect the first available to our input port
	
	
		if (jack_connect (client, serverports_names[0], jack_port_name (input_port1))) {
		printf("Cannot connect input port.\n");
		exit (1);
	   	}
	   	if (jack_connect (client, serverports_names[1], jack_port_name (input_port2))) {
		printf("Cannot connect input port.\n");
		exit (1);
	   	}
   	
	// free serverports_names variable for reuse in next part of the code
	free (serverports_names);
	
	
	/* Assign our output port to a server input port*/
	// Find possible input server port names
	serverports_names = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
	if (serverports_names == NULL) {
		printf("No available physical playback (server input) ports.\n");
		exit (1);
	}
	// Connect the first available to our output port
	
		if (jack_connect (client, jack_port_name (output_port), serverports_names[0])) {
			printf ("Cannot connect output ports.\n");
			exit (1);
		}
		
	
	// free serverports_names variable, we're not going to use it again
	free (serverports_names);
	
	
	printf ("done.\n");
	/* keep running until stopped by the user */
	sleep (-1);
	
	
	/* this is never reached but if the program
	   had some other way to exit besides being killed,
	   they would be important to call.
	*/
	jack_client_close (client);
	exit (0);
}
