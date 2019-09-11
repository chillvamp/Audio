/**
 * A simple example of how to do FFT with FFTW3 and JACK.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <jack/jack.h>

// Include FFTW header
#include <complex.h> //needs to be included before fftw3.h for compatibility
#include <fftw3.h>

double complex *i_fft, *i_time, *o_fft, *o_time;
double complex *i_fft_1, *i_time_1, *o_fft_1, *o_time_1;
double complex *i_fft_2, *i_time_2, *o_fft_2, *o_time_2;
double complex *i_fft_3, *i_time_3, *o_fft_3, *o_time_3;
double complex *i_fft_4, *i_time_4, *o_fft_4, *o_time_4;
double complex *i_fft_5, *i_time_5, *o_fft_5, *o_time_5;

fftw_plan i_forward, o_inverse;
fftw_plan i_forward_1, o_inverse_1;
fftw_plan i_forward_2, o_inverse_2;
fftw_plan i_forward_3, o_inverse_3;
fftw_plan i_forward_4, o_inverse_4;
fftw_plan i_forward_5, o_inverse_5;


jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;
jack_default_audio_sample_t *buff_a ,*buff_b,*buff_c,*buff_d,*buff_e ;
double sample_rate;
double *freqs;
double *hann;
double complex aux;

/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by 
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */
int jack_callback (jack_nframes_t nframes, void *arg){
	jack_default_audio_sample_t *in, *out;
	int i;
	
	in = (jack_default_audio_sample_t *)jack_port_get_buffer (input_port, nframes);
	out = (jack_default_audio_sample_t *)jack_port_get_buffer (output_port, nframes);
	

	
	// Obteniendo la transformada de Fourier de este periodo
	for(i = 0; i < nframes; i++){

		if (i>=nframes/2){
		 buff_a[i]= in[i-nframes/2];
							}
		
	i_time_1[i]= buff_a[i] * hann[i]; 
	i_time_2[i]= buff_b[i] * hann[i];
	i_time_3[i]= buff_c[i] * hann[i];
	i_time_4[i]= buff_d[i] * hann[i];
	i_time_5[i]= buff_e[i] * hann[i];
	
	buff_e[i]=buff_c[i];
	buff_d[i]=buff_b[i];	
	buff_c[i]=buff_a[i];
	buff_b[i]=in[i];

	}


	fftw_execute(i_forward_1);
	fftw_execute(i_forward_2);
	fftw_execute(i_forward_3);
	fftw_execute(i_forward_4);
	fftw_execute(i_forward_5);
	
	
	// Aquí podriamos hacer algo con i_fft
	for(i = 0; i < nframes; i++){
		aux= cexp (- I*2*freqs[i] *3.1416*-.0125);
				
		o_fft_1[i] = i_fft_1[i]* aux ;
		o_fft_2[i] = i_fft_2[i]* aux ;
		o_fft_3[i] = i_fft_3[i]* aux ;
		o_fft_4[i] = i_fft_4[i]* aux ;
		o_fft_5[i] = i_fft_5[i]* aux ;
		//printf ("OFFT [%f + %fj], IFFT[%f]+j[%f]\n",creal(o_fft[i]),cimag(o_fft[i]),creal(i_fft[i]),cimag(i_fft[i]) );
	}
	
	// Regresando al dominio del tiempo
	fftw_execute(o_inverse_1);
	fftw_execute(o_inverse_2);
	fftw_execute(o_inverse_3);
	fftw_execute(o_inverse_4);
	fftw_execute(o_inverse_5);




	for(i = 0; i < nframes; i++){
		out[i] = creal(o_time[i])/nframes; //fftw3 requiere normalizar su salida real de esta manera
		
	}
	
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
	const char *client_name = "jack_fft";
	jack_options_t options = JackNoStartServer;
	jack_status_t status;
	
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
	printf ("Sample rate: %d\n", jack_get_sample_rate (client));
	printf ("Window size: %d\n", jack_get_buffer_size (client));

	sample_rate = (double)jack_get_sample_rate(client);
	int nframes = jack_get_buffer_size (client);

	//CALCULATING fRC ARRAY
	freqs=  (double*) malloc (sizeof(double)*nframes);
	buff_a=(jack_default_audio_sample_t*) malloc (sizeof(jack_default_audio_sample_t)*nframes);
	buff_b=(jack_default_audio_sample_t*) malloc (sizeof(jack_default_audio_sample_t)*nframes);
	buff_c=(jack_default_audio_sample_t*) malloc (sizeof(jack_default_audio_sample_t)*nframes);
	buff_d=(jack_default_audio_sample_t*) malloc (sizeof(jack_default_audio_sample_t)*nframes);
	buff_e=(jack_default_audio_sample_t*) malloc (sizeof(jack_default_audio_sample_t)*nframes);

	for (int i=0;i<=nframes/2;++i){
		freqs[i]= i*(sample_rate/nframes);
		freqs [nframes-i]= -i*(sample_rate/nframes);

	
	}
	printf("MINFRE: %f; max Freq: %f\n",freqs[1], freqs[nframes/2]);
	hann=(double*) malloc (sizeof(double)*nframes);
	for (int i=0;i<nframes;++i){
		
		hann[i]= .5*(1-cos(2*M_PI*i/nframes));
		
	}
	//preparing FFTW3 buffers
	i_fft = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	i_time = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_fft = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_time = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	i_forward = fftw_plan_dft_1d(nframes, i_time, i_fft , FFTW_FORWARD, FFTW_MEASURE);
	o_inverse = fftw_plan_dft_1d(nframes, o_fft , o_time, FFTW_BACKWARD, FFTW_MEASURE);


	i_fft_1 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	i_time_1 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_fft_1 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_time_1 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	
	i_forward_1 = fftw_plan_dft_1d(nframes, i_time_1, i_fft_1 , FFTW_FORWARD, FFTW_MEASURE);
	o_inverse_1 = fftw_plan_dft_1d(nframes, o_fft_1 , o_time_1, FFTW_BACKWARD, FFTW_MEASURE);

	i_fft_2 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	i_time_2 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_fft_2 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_time_2 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	
	i_forward_2 = fftw_plan_dft_1d(nframes, i_time_2, i_fft_2 , FFTW_FORWARD, FFTW_MEASURE);
	o_inverse_2 = fftw_plan_dft_1d(nframes, o_fft_2 , o_time_2, FFTW_BACKWARD, FFTW_MEASURE);

	i_fft_3 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	i_time_3 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_fft_3 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_time_3 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	
	i_forward_3 = fftw_plan_dft_1d(nframes, i_time_3, i_fft_3 , FFTW_FORWARD, FFTW_MEASURE);
	o_inverse_3 = fftw_plan_dft_1d(nframes, o_fft_3 , o_time_3, FFTW_BACKWARD, FFTW_MEASURE);

	i_fft_4 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	i_time_4 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_fft_4 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_time_4 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	
	i_forward_4 = fftw_plan_dft_1d(nframes, i_time_4, i_fft_4 , FFTW_FORWARD, FFTW_MEASURE);
	o_inverse_4 = fftw_plan_dft_1d(nframes, o_fft_4 , o_time_4, FFTW_BACKWARD, FFTW_MEASURE);


	i_fft_5 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	i_time_5 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_fft_5 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	o_time_5 = (double complex *) fftw_malloc(sizeof(double complex) * nframes);
	
	i_forward_5 = fftw_plan_dft_1d(nframes, i_time_5, i_fft_5 , FFTW_FORWARD, FFTW_MEASURE);
	o_inverse_5 = fftw_plan_dft_1d(nframes, o_fft_5 , o_time_5, FFTW_BACKWARD, FFTW_MEASURE);









	
	/* create the agent input port */
	input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput, 0);
	
	/* create the agent output port */
	output_port = jack_port_register (client, "output",JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput, 0);
	
	/* check that both ports were created succesfully */
	if ((input_port == NULL) || (output_port == NULL)) {
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
	if (jack_connect (client, serverports_names[0], jack_port_name (input_port))) {
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
