/*
 * \brief  Startup code for Ada main program
 * \author Alexander Senier
 * \date   2019-01-03
 */

#include <ada/component.h>

extern "C" void _ada_main(void);

void Ada::Component::main()
{
    _ada_main();
}
