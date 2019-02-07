#ifndef _INCLUDE__ADA__COMPONENT_H_
#define _INCLUDE__ADA__COMPONENT_H_

#include <libc/component.h>

namespace Ada { namespace Component {
	/**
	 * Run ada main program
	 */
	void main(void);
} }

namespace Libc { namespace Component {

   extern "C" void adainit(void);
   extern "C" void adafinal(void);

   extern "C" void __gnat_runtime_initialize(void) { };
   extern "C" void __gnat_runtime_finalize(void) { };
   extern "C" int gnat_exit_status;

	/**
	 * Construct component
	 *
	 * \param env  extended interface to the component's execution environment
	 */
	void construct(Libc::Env &env) {
      Libc::with_libc([&] () {
         adainit();
         Ada::Component::main();
         adafinal();
      });
      env.parent().exit(gnat_exit_status);
   }
} }

#endif /* _INCLUDE__ADA__COMPONENT_H_ */
