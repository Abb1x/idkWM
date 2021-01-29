#include "idkWM/idkWM.hpp"

int main()
{
  idkWM::wm::get()->init();
  idkWM::wm::get()->run();
  idkWM::wm::get()->exit();
  return 0;
}
