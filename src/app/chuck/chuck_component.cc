/*----------------------------------------------------------------------------
 ChucK Concurrent, On-the-fly Audio Programming Language
 Compiler and Virtual Machine
 
 Copyright (c) 2003 Ge Wang and Perry R. Cook.  All rights reserved.
 http://chuck.stanford.edu/
 http://chuck.cs.princeton.edu/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 U.S.A.
 -----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// file: component.cpp
// desc: chuck entry point for Genode
//-----------------------------------------------------------------------------

/* Genode includes */
#include <audio_out_session/audio_out_session.h>
#include <timer_session/connection.h>
#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <base/log.h>

/* ChucK includes */
#include "chuck.h"
#include "chuck_audio.h"

using namespace Genode;


//-----------------------------------------------------------------------------
// global variables
//-----------------------------------------------------------------------------
// the one ChucK, for command line host
ChucK * the_chuck;


//-----------------------------------------------------------------------------
// name: cb()
// desc: audio callback
//-----------------------------------------------------------------------------
void cb( t_CKSAMPLE * in, t_CKSAMPLE * out, t_CKUINT numFrames,
        t_CKUINT numInChans, t_CKUINT numOutChans, void * data )
{
    // TODO: check channel numbers
    
    // call up to ChucK
    the_chuck->run( in, out, numFrames );
}


struct Main
{
	Libc::Env &env;

	t_CKBOOL g_enable_realtime_audio = TRUE;
	t_CKBOOL enable_system_cmd = FALSE;

	Main(Libc::Env &env): env(env) { };

	void go()
	{
		t_CKBOOL vm_halt = TRUE;
		t_CKINT srate = SAMPLE_RATE_DEFAULT;
		t_CKBOOL force_srate = FALSE; // added 1.3.1.2
		t_CKINT buffer_size = BUFFER_SIZE_DEFAULT;
		t_CKINT num_buffers = NUM_BUFFERS_DEFAULT;
		t_CKINT dac = 0;
		t_CKINT adc = 0;
		std::string dac_name = ""; // added 1.3.0.0
		std::string adc_name = ""; // added 1.3.0.0
		t_CKINT dac_chans = 0;
		t_CKINT adc_chans = 2;
		t_CKBOOL dump = FALSE;
		t_CKBOOL auto_depend = FALSE;
		t_CKBOOL block = FALSE;
		// t_CKBOOL enable_shell = FALSE;
		t_CKBOOL no_vm = FALSE;
		t_CKBOOL load_hid = FALSE;
		t_CKBOOL enable_server = TRUE;
		t_CKBOOL do_watchdog = TRUE;
		t_CKINT  adaptive_size = 0;
		t_CKINT  log_level = CK_LOG_CORE;
		t_CKINT  deprecate_level = 1; // 1 == warn
		t_CKINT  chugin_load = 1; // 1 == auto (variable added 1.3.0.0)
		// whether to make this new VM the one that receives OTF commands
		t_CKBOOL update_otf_vm = TRUE;
		string   filename = "";
		vector<string> args;

		// list of search pathes (added 1.3.0.0)
		std::list<std::string> dl_search_path;
		// initial chug-in path (added 1.3.0.0)
		std::string initial_chugin_path;
		// if set as environment variable (added 1.3.0.0)
		if( getenv( g_chugin_path_envvar ) )
		{
			// get it from the env var
			initial_chugin_path = getenv( g_chugin_path_envvar );
		}
		else
		{
			// default it
			initial_chugin_path = g_default_chugin_path;
		}
		// parse the colon list into STL list (added 1.3.0.0)
		parse_path_list( initial_chugin_path, dl_search_path );
		// list of individually named chug-ins (added 1.3.0.0)
		std::list<std::string> named_dls;

	#if defined(__DISABLE_WATCHDOG__)
		do_watchdog = FALSE;
	#elif defined(__MACOSX_CORE__)
		do_watchdog = TRUE;
	#elif defined(__PLATFORM_WIN32__) && !defined(__WINDOWS_PTHREAD__)
		do_watchdog = TRUE;
	#else
		do_watchdog = FALSE;
	#endif

		t_CKUINT files = 0;
		t_CKUINT count = 1;
		t_CKINT i;

		// set log level
		EM_setlog( log_level );

		// add myself to the list of Chuck_Systems that might need to be cleaned up
		// g_systems.push_back( this );


		//------------------------- COMMAND LINE ARGUMENTS -----------------------------

		// log level
		EM_setlog( log_level );

		// set caution to wind
		ChucK::enableSystemCall = enable_system_cmd;

		// check buffer size
		buffer_size = ensurepow2( buffer_size );
		// set watchdog
		g_do_watchdog = do_watchdog;
		// set adaptive size
		if( adaptive_size < 0 ) adaptive_size = buffer_size;

		// make sure vm
		if( no_vm )
		{
			CK_FPRINTF_STDERR( "[chuck]: '--empty' can only be used with shell...\n" );
			exit( 1 );
		}

		// find dac_name if appropriate (added 1.3.0.0)
		if( dac_name.size() > 0 )
		{
			// check with RtAudio
			int dev = ChuckAudio::device_named( dac_name, TRUE, FALSE );
			if( dev >= 0 )
			{
				dac = dev;
			}
			else
			{
				CK_FPRINTF_STDERR( "[chuck]: unable to find dac '%s'...\n", dac_name.c_str() );
				exit( 1 );
			}
		}

		// find adc_name if appropriate (added 1.3.0.0)
		if( adc_name.size() > 0 )
		{
			// check with RtAudio
			int dev = ChuckAudio::device_named( adc_name, FALSE, TRUE );
			if( dev >= 0 )
			{
				adc = dev;
			}
			else
			{
				CK_FPRINTF_STDERR( "[chuck]: unable to find adc '%s'...\n", adc_name.c_str() );
				exit( 1 );
			}
		}

		//------------------------- VIRTUAL MACHINE SETUP -----------------------------
		// instantiate ChucK
		the_chuck = new ChucK();

		// set params
		the_chuck->setParam( CHUCK_PARAM_SAMPLE_RATE, Audio_out::SAMPLE_RATE );
		the_chuck->setParam( CHUCK_PARAM_INPUT_CHANNELS, 0 );
		the_chuck->setParam( CHUCK_PARAM_OUTPUT_CHANNELS, 2 );
		the_chuck->setParam( CHUCK_PARAM_VM_ADAPTIVE, adaptive_size );
		the_chuck->setParam( CHUCK_PARAM_VM_HALT, (t_CKINT)(vm_halt) );
		the_chuck->setParam( CHUCK_PARAM_OTF_ENABLE, (t_CKINT)FALSE );
		the_chuck->setParam( CHUCK_PARAM_DUMP_INSTRUCTIONS, (t_CKINT)dump );
		the_chuck->setParam( CHUCK_PARAM_AUTO_DEPEND, (t_CKINT)auto_depend );
		the_chuck->setParam( CHUCK_PARAM_DEPRECATE_LEVEL, deprecate_level );
		the_chuck->setParam( CHUCK_PARAM_USER_CHUGINS, named_dls );
		the_chuck->setParam( CHUCK_PARAM_USER_CHUGIN_DIRECTORIES, dl_search_path );
		// set hint, so internally can advise things like async data writes etc.
		the_chuck->setParam( CHUCK_PARAM_HINT_IS_REALTIME_AUDIO, TRUE );
		the_chuck->setLogLevel( log_level );

		// initialize
		if( !the_chuck->init() )
		{
			CK_FPRINTF_STDERR( "[chuck]: failed to initialize...\n" );
			exit( 1 );
		}

		//--------------------------- AUDIO I/O SETUP ---------------------------------
		// log
		EM_log( CK_LOG_SYSTEM, "initializing audio I/O..." );
		// push
		EM_pushlog();
		// log

		// initialize audio system
		// TODO: refactor initialize() to take in the dac and adc nums
		ChuckAudio::m_adc_n = adc;
		ChuckAudio::m_dac_n = dac;
		t_CKBOOL retval = ChuckAudio::initialize( adc_chans, dac_chans,
			srate, buffer_size, num_buffers, cb, (void *)the_chuck, force_srate );
		// check
		if( !retval )
		{
			EM_log( CK_LOG_SYSTEM,
				   "cannot initialize audio device (use --silent/-s for non-realtime)" );
			// pop
			EM_poplog();
			// done
			exit( 1 );
		}

		// log
		EM_log( CK_LOG_SYSTEM, "real-time audio: %s", g_enable_realtime_audio ? "YES" : "NO" );
		EM_log( CK_LOG_SYSTEM, "mode: %s", block ? "BLOCKING" : "CALLBACK" );
		EM_log( CK_LOG_SYSTEM, "sample rate: %ld", srate );
		EM_log( CK_LOG_SYSTEM, "buffer size: %ld", buffer_size );
		if( g_enable_realtime_audio )
		{
			EM_log( CK_LOG_SYSTEM, "num buffers: %ld", num_buffers );
			EM_log( CK_LOG_SYSTEM, "adc: %ld dac: %d", adc, dac );
			EM_log( CK_LOG_SYSTEM, "adaptive block processing: %ld", adaptive_size > 1 ? adaptive_size : 0 );
		}
		EM_log( CK_LOG_SYSTEM, "channels in: %ld out: %ld", adc_chans, dac_chans );

		// pop
		EM_poplog();

		// reset count
		count = 1;

		// log
		EM_log( CK_LOG_SEVERE, "starting compilation..." );
		// push indent
		EM_pushlog();


		//------------------------- SOURCE COMPILATION --------------------------------

		// loop through and process each file
		auto const arg_fn = [&] (Xml_node const &node) {
			auto const val = node.attribute_value("value", String<256>());
			char const *arg = val.string();
			// make sure
			if( arg[0] == '-' || arg[0] == '+' )
			{
				if( val == "--dump" || val == "+d" )
					the_chuck->compiler()->emitter->dump = TRUE;
				else if( val == "--nodump" || val == "-d" )
					the_chuck->compiler()->emitter->dump = FALSE;
				return;
			}

			// compile it!
			the_chuck->compileFile( arg, "" );
			++files;
		};

		env.config([&] (Xml_node const &config) {
			config.for_each_sub_node("arg", arg_fn); });

		if( !files && vm_halt)
		{
			CK_FPRINTF_STDERR( "[chuck]: no input files... (try --help)\n" );
			exit( 1 );
		}

		// pop indent
		EM_poplog();


		//-------------------------- MAIN CHUCK LOOP!!! -----------------------------

		// log
		EM_log( CK_LOG_SYSTEM, "running main loop..." );
		// push indent
		EM_pushlog();

		// start it!
		the_chuck->start();

		// log
		EM_log( CK_LOG_SEVERE, "virtual machine running..." );
		// pop indent
		EM_poplog();

		// silent mode buffers
		SAMPLE * input = new SAMPLE[buffer_size*adc_chans];
		SAMPLE * output = new SAMPLE[buffer_size*dac_chans];
		// zero out
		Genode::memset( input, 0, sizeof(SAMPLE)*buffer_size*adc_chans );
		Genode::memset( output, 0, sizeof(SAMPLE)*buffer_size*dac_chans );

		// start audio
		ChuckAudio::start();

		// return to entrypoint
	}
};


void Libc::Component::construct(Libc::Env &env)
{
	init_rtaudio(env);

	static Main main(env);

	Libc::with_libc([&] () { main.go(); });
}

extern "C" {
	int pthread_setschedparam(pthread_t, int, const struct sched_param*) {
		return 0; }
	int pthread_getschedparam(pthread_t, int*, struct sched_param*) {
		return 0; }
}
