#include "client_if.hpp"
#include "vp_engine.hpp"


int main(){
    VPEngine engine;
    ClientIF cif;

    thread t1(&VPEngine::run, &engine);
    thread t2(&ClientIF::run, &cif);

    t1.join();
    t2.join();

    return 0;
}