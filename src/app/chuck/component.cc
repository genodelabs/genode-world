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
#include "chuck_compile.h"
#include "chuck_vm.h"
#include "chuck_bbq.h"
#include "chuck_errmsg.h"
#include "chuck_lang.h"
#include "chuck_console.h"
#include "chuck_globals.h"

#include "util_math.h"
#include "util_string.h"
#include "util_thread.h"
#include "ulib_machine.h"
#include "chuck_system.h"

using namespace Genode;

struct Main : Chuck_System
{
	Genode::Env &env;

	Attached_rom_dataspace config_rom { env, "config" };

	Timer::Connection timer;

	void load_config()
	{
		/* compile the config in sequence */
		config_rom.xml().for_each_sub_node([&] (Xml_node const node) {
			if (node.has_type("file"))
				try {
					Xml_attribute path_attr = node.attribute("path");
					std::string path(path_attr.value_base(), path_attr.value_size());
					std::string args;
					if (node.has_attribute("args")) {
						Xml_attribute args_attr = node.attribute("args");
						args = std::string(args_attr.value_base(), args_attr.value_size());
					}
					if (!compileFile(path, args))
						error("failed to compile ", path.c_str());
					else
						log("compiled ", path.c_str());
				} catch (...) {
					error("failed to parse file node");
				}

			else if (node.has_type("code"))
				try {
					Xml_attribute path_attr = node.attribute("path");
					std::string code(node.content_base(), node.content_size());
					std::string args;
					if (node.has_attribute("args")) {
						Xml_attribute args_attr = node.attribute("args");
						args = std::string(args_attr.value_base(), args_attr.value_size());
					}
					if (!compileCode(code, args))
						error("compilation failed");
					else
						log("compiled ", code.c_str());
				} catch (...) {
					error("failed to parse code node");
				}
		});
	}

	void handle_config()
	{
		config_rom.update();
		load_config();
	}

	//Signal_handler<Main> config_handler
	//	{ env.ep(), *this, &Main::handle_config };

	void handle_timeout()
	{
		if (!g_vm->running()) {
			env.parent().exit(0);
			timer.trigger_periodic(0);
		} else if( g_main_thread_hook && g_main_thread_quit ) {
			log("bindling something");
			g_main_thread_hook( g_main_thread_bindle );
		}
	}

	Signal_handler<Main> timeout_handler
		{ env.ep(), *this, &Main::handle_timeout };

	Main(Genode::Env &env);
};

Main::Main(Genode::Env &env) : env(env)
{
	Genode::Xml_node config_node = config_rom.xml();

	Chuck_Compiler * compiler = NULL;
	Chuck_VM * vm = NULL;
	Chuck_VM_Code * code = NULL;
	Chuck_VM_Shred * shred = NULL;
	// ge: refactor 2015
	BBQ * bbq = NULL;
	t_CKBOOL audio_started = FALSE;

	t_CKBOOL vm_halt = TRUE;
	t_CKUINT srate = Audio_out::SAMPLE_RATE;
	t_CKBOOL force_srate = FALSE; // added 1.3.1.2
	t_CKUINT buffer_size = BUFFER_SIZE_DEFAULT;
	t_CKUINT num_buffers = NUM_BUFFERS_DEFAULT;
	t_CKUINT dac = 0;
	t_CKUINT adc = 0;
	std::string dac_name = ""; // added 1.3.0.0
	std::string adc_name = ""; // added 1.3.0.0

	t_CKUINT dac_chans = config_node.attribute_value("dac_channels", 2UL);
	t_CKUINT adc_chans = config_node.attribute_value("adc_channels", 0UL);

	t_CKBOOL dump = FALSE;
	t_CKBOOL probe = FALSE;
	t_CKBOOL set_priority = FALSE;
	t_CKBOOL auto_depend = FALSE;
	t_CKBOOL block = FALSE;
	t_CKBOOL no_vm = FALSE;
	t_CKBOOL load_hid = FALSE;
	t_CKBOOL enable_server = FALSE;
	t_CKBOOL do_watchdog = TRUE;
	t_CKINT  adaptive_size = 0;
	t_CKINT  log_level = CK_LOG_INFO;
	t_CKINT  deprecate_level = 1; // 1 == warn
	t_CKINT  chugin_load = 1; // 1 == auto (variable added 1.3.0.0)
	string   filename = "";
	vector<string> args;

	// list of search pathes (added 1.3.0.0)
	std::list<std::string> dl_search_path;
	// initial chug-in path (added 1.3.0.0)
	std::string initial_chugin_path;

	// default it
	initial_chugin_path = g_default_chugin_path;

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

	g_enable_realtime_audio = TRUE;

	// set adaptive size
	if( adaptive_size < 0 ) adaptive_size = buffer_size;


//------------------------- VIRTUAL MACHINE SETUP -----------------------------

	// allocate the vm - needs the type system
	vm = m_vmRef = g_vm = new Chuck_VM;
	// ge: refactor 2015: initialize VM
	if( !vm->initialize( srate, dac_chans, adc_chans, adaptive_size, vm_halt ) )
	{
		fprintf( stderr, "[chuck]: %s\n", vm->last_error() );
		env.parent().exit( 1 );
	}


//--------------------------- AUDIO I/O SETUP ---------------------------------

	// ge: 1.3.5.3
	bbq = g_bbq = new BBQ;
	// set some parameters
	bbq->set_srate( srate );
	bbq->set_bufsize( buffer_size );
	bbq->set_numbufs( num_buffers );
	bbq->set_inouts( adc, dac );
	bbq->set_chans( adc_chans, dac_chans );

	// log
	EM_log( CK_LOG_SYSTEM, "initializing audio I/O..." );
	// push
	EM_pushlog();
	// log
	EM_log( CK_LOG_SYSTEM, "probing '%s' audio subsystem...", g_enable_realtime_audio ? "real-time" : "fake-time" );

	// probe / init (this shouldn't start audio yet...
	// moved here 1.3.1.2; to main ge: 1.3.5.3)
	if( !bbq->initialize( dac_chans, adc_chans, srate, 16, buffer_size, num_buffers,
						  dac, adc, block, vm, g_enable_realtime_audio, NULL, NULL, force_srate ) )
	{
		EM_log( CK_LOG_SYSTEM,
				"cannot initialize audio device" );
		// pop
		EM_poplog();
		// done
		env.parent().exit( 1 );
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


//------------------------- CHUCK COMPILER SETUP -----------------------------

	// if chugin load is off, then clear the lists (added 1.3.0.0 -- TODO: refactor)
	if( chugin_load == 0 )
	{
		// turn off chugin load
		dl_search_path.clear();
		named_dls.clear();
	}

	// allocate the compiler
	compiler = m_compilerRef= g_compiler = new Chuck_Compiler;
	// initialize the compiler (search_apth and named_dls added 1.3.0.0 -- TODO: refactor)
	if( !compiler->initialize( vm, dl_search_path, named_dls ) )
	{
		fprintf( stderr, "[chuck]: error initializing compiler...\n" );
		env.parent().exit( 1 );
	}
	// enable dump
	compiler->emitter->dump = dump;
	// set auto depend
	compiler->set_auto_depend( auto_depend );

	// vm synthesis subsystem - needs the type system
	if( !vm->initialize_synthesis( ) )
	{
		fprintf( stderr, "[chuck]: %s\n", vm->last_error() );
		env.parent().exit( 1 );
	}


	// set deprecate
	compiler->env->deprecate_level = deprecate_level;

	// reset count
	count = 1;

	compiler->env->load_user_namespace();

	// log
	EM_log( CK_LOG_SEVERE, "starting compilation..." );
	// push indent
	EM_pushlog();


//------------------------- SOURCE COMPILATION --------------------------------

	// loop through and process each file
	load_config();

	// pop indent
	EM_poplog();

	// reset the parser
	reset_parse();

	// boost priority
	if( Chuck_VM::our_priority != 0x7fffffff )
	{
		// try
		if( !Chuck_VM::set_priority( Chuck_VM::our_priority, vm ) )
		{
			// error
			fprintf( stderr, "[chuck]: %s\n", vm->last_error() );
			env.parent().exit( 1 );
		}
	}

//-------------------------- MAIN CHUCK LOOP!!! -----------------------------

	// log
	EM_log( CK_LOG_SYSTEM, "running main loop..." );
	// push indent
	EM_pushlog();

	// set run state
	vm->start();

	EM_log( CK_LOG_SEVERE, "initializing audio buffers..." );
	if( !bbq->digi_out()->initialize( ) )
	{
		EM_log( CK_LOG_SYSTEM,
			   "cannot open audio output (use --silent/-s)" );
		env.parent().exit(1);
	}

	// initialize input
	bbq->digi_in()->initialize( );

	// log
	EM_log( CK_LOG_SEVERE, "virtual machine running..." );
	// pop indent
	EM_poplog();

	// NOTE: non-blocking callback only, ge: 1.3.5.3

	// compute shreds before first sample
	if( !vm->compute() )
	{
		// done, 1.3.5.3
		vm->stop();
		// log
		EM_log( CK_LOG_SYSTEM, "virtual machine stopped..." );
	}

	// start audio
	if( !audio_started )
	{
		// audio
		if( !audio_started && g_enable_realtime_audio )
		{
			EM_log( CK_LOG_SEVERE, "starting real-time audio..." );
			bbq->digi_out()->start();
			bbq->digi_in()->start();
		}

		// set the flag to true to avoid entering this function
		audio_started = TRUE;
	}


	// silent mode buffers
	SAMPLE * input  = new SAMPLE[buffer_size*adc_chans];
	SAMPLE * output = new SAMPLE[buffer_size*dac_chans];
	// zero out
	Genode::memset( input, 0, sizeof(SAMPLE)*buffer_size*adc_chans );
	Genode::memset( output, 0, sizeof(SAMPLE)*buffer_size*dac_chans );

	//timer.sigh(timeout_handler);
	//timer.trigger_periodic(100000);
};


void Libc::Component::construct(Genode::Env &env)
{
	static Main inst(env);
}
