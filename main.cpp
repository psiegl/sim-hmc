#include "main.h"
#include "crossbar.h"
#include "hmc_link.h"

int main(int argc, char* argv[])
{
  unsigned linkdepth = 5;
  unsigned linkwidth = 64;

  notify notifier;
  hmc_xbar<notify> crossbar(0);
  crossbar.set_notify( &notifier, &notify::add, &notify::del );
  hmc_link<hmc_xbar<notify>, hmc_xbar<notify>> link;

  crossbar.connect_xbar_link( 1, &link );
  crossbar.re_adjust_xbar_link( linkwidth, linkdepth );

  link.re_adjust( linkwidth, linkdepth );
  link.set_olink( crossbar.get_xbar_link(1)->get_ilink() );

  unsigned packetlen= 128;
  void *packet = (void*) malloc (packetlen);
  link.get_olink()->push_back( packet, packetlen );

  std::cout << "done" << std::endl;
  return 0;
}
