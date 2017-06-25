/* Genode includes */
#include <libc/component.h>

/* libc includes */
#include <stdlib.h> /* 'exit'   */

/* provided by the application */
extern "C" int main(int argc, char const **argv);

void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] {
		int argc = 1;
		char const *argv[] = {
			"gtest-samples",
			0
		};
		exit(main(argc, argv));
	});
}
