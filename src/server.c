#include "engine.h"

int main(int argc, const char** argv) {
    turbo_engine_t* engine = turbo_engine_create("0.0.0.0", 45678, 0);
    turbo_engine_run(engine);
    turbo_engine_destroy(&engine);
}
