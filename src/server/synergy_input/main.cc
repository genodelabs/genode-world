/*
 * \brief  Synergy client
 * \author Emery Hemingway
 * \date   2015-06-14
 *
 * http://synergy-project.org/
 */

/*
 * Copyright (C) 2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/log.h>
#include <input/component.h>
#include <framebuffer_session/connection.h>
#include <nitpicker_session/connection.h>
#include <timer_session/connection.h>
#include <os/static_root.h>
#include <base/attached_rom_dataspace.h>
#include <libc/component.h>

/* Synergy includes */
#include <uSynergy.h>

/* socket API */
extern "C" {
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
}

using namespace Genode;


Constructible<Attached_rom_dataspace> config;
Constructible<Timer::Connection>      timer;


struct Session_component : Input::Session_component
{
	/* Array for tracking the current keyboard state */
	bool                                 key_state[Input::KEY_MAX + 1];
	int                                  socket_fd;
	uSynergyBool                         button_left;
	uSynergyBool                         button_right;
	uSynergyBool                         button_middle;

	Session_component(Genode::Env &env)
	: Input::Session_component(env, env.ram()), socket_fd(-1) { }

	~Session_component()
	{
		::close(socket_fd);
	}

	void reset_keys()
	{
		for (int i = 0; i <= Input::KEY_MAX; i++)
		key_state[i] = false;
		button_left  = USYNERGY_FALSE;
		button_right = USYNERGY_FALSE;
		button_right = USYNERGY_FALSE;
	}

};


/***********************
 ** Synergy callbacks **
 ***********************/

static uSynergyBool connect(uSynergyCookie cookie)
{
	Session_component *session = (Session_component*)cookie;

	/******************
	 ** Parse config **
	 ******************/

	char addr[INET_ADDRSTRLEN];
	unsigned long port = 24800;

	Xml_node config_node = config->xml();

	try {
		config_node.attribute("addr").value(addr, sizeof(addr));
	} catch (Xml_node::Nonexistent_attribute) {
		Genode::error("server address not set in config");
		return USYNERGY_FALSE;
	}

	try {
		config_node.attribute("port").value(&port);
	} catch (...) { }


	/*****************************
	 ** Open and connect socket **
	 *****************************/

	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port   = htons(port);
	if (inet_pton(AF_INET, addr, &sockaddr.sin_addr.s_addr) == 0) {
		Genode::error("bad IPv4 address ", Cstring(addr), " for server");
		return USYNERGY_FALSE;
	}

	session->socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (session->socket_fd == -1)
		return USYNERGY_FALSE;

	if (::connect(session->socket_fd, (struct sockaddr*) &sockaddr, sizeof(sockaddr))) {
		::close(session->socket_fd);
		return USYNERGY_FALSE;
	}
	return USYNERGY_TRUE;
}

uSynergyBool send(uSynergyCookie cookie, const uint8_t *buffer, int length)
{
	Session_component *session = (Session_component*)cookie;

	return (length == ::write(session->socket_fd, buffer, length)) ?
		USYNERGY_TRUE : USYNERGY_FALSE;
}

uSynergyBool receive(uSynergyCookie cookie, uint8_t *buffer, int maxLength, int* outLength)
{
	Session_component *session = (Session_component*)cookie;

	*outLength = ::read(session->socket_fd, buffer, maxLength);
	if (!*outLength)
		return USYNERGY_FALSE;

	return USYNERGY_TRUE;
}

void sleep(uSynergyCookie, int timeMs) { timer->msleep(timeMs); }

uint32_t get_time() { return timer->elapsed_ms(); }

void trace_callback(uSynergyCookie cookie, const char *text) { Genode::log(text); }

void screen_active_callback(uSynergyCookie cookie, uSynergyBool active)
{
	if (!active) {
		Session_component *session = (Session_component*)cookie;
		Input::Event_queue &queue = session->event_queue();
		queue.reset();
		queue.add(Input::Event(Input::Event::LEAVE, 0, 0, 0, 0, 0));
		session->reset_keys();
	}
}

void mouse_callback(uSynergyCookie cookie,
                    uint16_t      x, uint16_t      y,
                    int16_t  wheelX, int16_t  wheelY, 
                    uSynergyBool buttonLeft,
                    uSynergyBool buttonRight,
                    uSynergyBool buttonMiddle)
{
	Session_component *session = (Session_component*)cookie;
	Input::Event_queue &queue = session->event_queue();

	if (queue.avail_capacity() < 5)
		queue.reset();

	/* Defer sending a signal until all conditions are processed */

	queue.add(Input::Event(Input::Event::MOTION, 0, x, y, 0, 0), false);
	queue.add(Input::Event(Input::Event::WHEEL,  0, wheelX, wheelY, 0, 0), false);

	if (buttonLeft != session->button_left)
		queue.add(Input::Event((session->button_left = buttonLeft) ?
			                   Input::Event::PRESS : Input::Event::RELEASE,
			                   Input::BTN_LEFT, 0, 0, 0, 0), false);

	if (buttonRight != session->button_right)
		queue.add(Input::Event((session->button_right = buttonRight) ?
			                   Input::Event::PRESS : Input::Event::RELEASE,
			                   Input::BTN_RIGHT, 0, 0, 0, 0), false);

	if (buttonMiddle != session->button_middle)
		queue.add(Input::Event((session->button_middle = buttonMiddle) ?
			                   Input::Event::PRESS : Input::Event::RELEASE,
			                   Input::BTN_MIDDLE, 0, 0, 0, 0), false);
	queue.submit_signal();
}

void keyboard_callback(uSynergyCookie cookie,
                       uint16_t     key,  uint16_t     modifiers,
                       uSynergyBool down, uSynergyBool repeat)
{
	Session_component *session = (Session_component*)cookie;
	Input::Event_queue &queue = session->event_queue();

	if (!queue.avail_capacity()) queue.reset();

	key -= 8; // TODO what is <8?
	if (key > Input::KEY_MAX) return;

	queue.add(Input::Event(((session->key_state[key] = !session->key_state[key]) ?
	                       Input::Event::PRESS : Input::Event::RELEASE),
	                       key, 0, 0, 0, 0));
}

/*
 * void joystick_callback(uSynergyCookie cookie,
 *                        uint8_t joyNum,
 *                        uint16_t buttons,
 *                        int8_t leftStickX,  int8_t leftStickY,
 *                        int8_t rightStickX, int8_t rightStickY);
 */


/*******************************
 ** Network processing thread **
 *******************************/

struct Synergy_thread : Thread
{

	enum {
		MAX_NAME_LEN = 256,
		STACK_SIZE   = 1024*sizeof(long)
	};

	Genode::Env &env;

	char            screen_name[MAX_NAME_LEN];
	uSynergyContext context;
	Signal_receiver config_rec;
	Signal_context  config_ctx;

	Synergy_thread(Genode::Env &env, Session_component &session)
	: Thread(env, "uSynergy", STACK_SIZE), env(env)
	{
		*screen_name = 0;
		uSynergyInit(&context);

		context.m_connectFunc  = &connect;    /* Connect function */
		context.m_sendFunc     = &send;       /* Send data function */
		context.m_receiveFunc  = &receive;    /* Receive data function */
		context.m_sleepFunc    = &sleep;      /* Thread sleep function */
		context.m_getTimeFunc  = &get_time;   /* Get current time function */
		context.m_clientName   = screen_name; /* Name of Synergy Screen */

		context.m_cookie = (uSynergyCookie)&session; /* Cookie pointer passed to callback functions (can be NULL) */
		context.m_traceFunc            = &trace_callback;         /* Function for tracing status (can be NULL) */
		context.m_screenActiveCallback = &screen_active_callback; /* Callback for entering and leaving screen */
		context.m_mouseCallback        = &mouse_callback;         /* Callback for mouse events */
		context.m_keyboardCallback     = &keyboard_callback;      /* Callback for keyboard events */

		config->sigh(config_rec.manage(&config_ctx));
	}

	~Synergy_thread()
	{
		config_rec.dissolve(&config_ctx);
	}

	/**
	 * Update configuration; return success state
	 */
	bool update_config()
	{
		/*
		 * TODO: detect changes to network config
		 * and trigger a reconnect if appropriate.
		 */
		Xml_node config_node = config->xml();

		try {
			config_node.attribute("addr");;
		} catch (Xml_node::Nonexistent_attribute) {
			Genode::error("server address not set in config");
			return false;
		}

		try {
			config_node.attribute("name").value(screen_name, sizeof(screen_name));
		} catch (Xml_node::Nonexistent_attribute) {
			Genode::error("client screen name not set in config, waiting for update");
			return false;
		}

		/*
		 * TODO: just get the capability for framebuffer or nitpicker,
		 * then make a simple resolution client that wraps that.
		 */

		Genode::log("probing Nitpicker service");
		try {
			Nitpicker::Connection conn { env, "dimension" };
			Framebuffer::Mode mode = conn.mode();

			context.m_clientWidth  = mode.width();
			context.m_clientHeight = mode.height();
			return true;
		} catch (...) { }

		Genode::log("probing Framebuffer service");
		try {
			Framebuffer::Connection conn { env, Framebuffer::Mode() };
			Framebuffer::Mode mode = conn.mode();

			context.m_clientWidth  = mode.width();
			context.m_clientHeight = mode.height();
			return true;
		} catch (...) { }

		/*
		 * No real pointer space, but give the server a small holding area.
		 *
		 * XXX: drop pointer events without a screen?
		 */
		Genode::log("using a virtual screen area");
		context.m_clientWidth  = context.m_clientHeight = 64;
		return true;
	}

	/**
	 * Parse the config, then spin on the Synergy update function.
	 *
	 * If the config is not valid, block until it is updated.
	 */
	void entry()
	{
		while (!update_config()) {
			config_rec.wait_for_signal();
			config->update();
		}

		for (;;) {
			uSynergyUpdate(&context);
			if (config_rec.pending()) {
				while (!update_config())
				config_rec.wait_for_signal();
				config->update();
			}
		}
	}
};


/******************
 ** Main program **
 ******************/

using namespace Genode;

struct Main
{
	Genode::Env &env;

	/*
	 * Input session provided to our client
	 */
	Session_component session_component { env };

	/*
	 * Attach root interface to the entry point
	 */
	Static_root<Input::Session> input_root { env.ep().manage(session_component) };

	/*
	 * Additional thread for processing incoming events.
	 */
 	Synergy_thread synergy_thread { env, session_component };

	/**
	 * Constructor
	 */
	Main(Genode::Env &env) : env(env)
	{
		session_component.event_queue().enabled(true);

		env.parent().announce(env.ep().manage(input_root));

		synergy_thread.start();		
	}
};


/***************
 ** Component **
 ***************/

void Libc::Component::construct(Libc::Env &env)
{
	config.construct(env, "config");
	timer.construct(env, "uSynergy");
	static Main inst(env);
}
