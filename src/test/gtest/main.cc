/* Genode includes */
#include <libc/component.h>

/* libc includes */
#include <stdlib.h> /* 'exit'   */

/* provided by the application */
extern "C" int main(int argc, char const **argv);

void Libc::Component::construct(Libc::Env &env)
{
	Libc::with_libc([&] {
		int argc = 2;
		char const *argv[] = {
			"gtest",
			"--gtest_filter=-OutputFile*:Directory*Test.*:*Death*:InitGoogleTestTest.*:CaptureTest.*:RETest*:Thread*:Mutex*:EXPECT_PRED1Test.Function*:ColoredOutputTest.UsesColorsWhenTermSupportsColors",
			0
		};
		exit(main(argc, argv));
	});
}
