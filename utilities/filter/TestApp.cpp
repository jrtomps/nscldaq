

#include <CFilterMain.h>
#include <CFilter.h>



int main(int argc, char** argv)
{

  // Create a transparent filter
  CFilter* filter = new CFilter;

  CFilterMain theApp(argc, argv);
  theApp.registerFilter(filter);

  theApp();

  return 0;

}
