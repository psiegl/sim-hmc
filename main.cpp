#include <iostream>
#include "hmc_notify.h"
#include "hmc_ring.h"
#include "hmc_link.h"


int main(int argc, char* argv[])
{
  unsigned linkdepth = 5;
  unsigned linkwidth = 64;

  hmc_notify notifier(0, nullptr);
  hmc_ring ring(0, &notifier);
  hmc_link link[2];
  link[0].connect_linkports(&link[1]);
  link[0].re_adjust_links( linkwidth, linkdepth );



  ring.set_ring_link( 1, &link[0] );


  unsigned packetlen= 128;
  void *packet = (void*) malloc (packetlen);
  link[1].get_olink()->push_back( packet, packetlen );

  for( unsigned i; i < 10; i++ ) {
    // set clk anyway
    if(notifier.get_notification())
    {
      ring.clock();
    }
  }

  std::cout << "done " << notifier.get_notification() << std::endl;
  return 0;
}
