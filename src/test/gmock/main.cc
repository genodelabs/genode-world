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
			"gmock",
			"--gtest_filter=-*Death*:*.*WhenVerbosityIs*:ExpectCallTest.TakesDefaultAction*:AssertTest.FailsFatally*:LogTest.*:*.LogsAnything*:*MockTest.*:*IsMandatory:*.DoesNotWarnOnAdequateActionCount:*.WarnsOn*:*.Reports*:*FunctionCallMessageTest.*:GMockVerboseFlagTest.*",
			0
		};
		exit(main(argc, argv));
	});
}
