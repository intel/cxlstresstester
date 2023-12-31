/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <unistd.h>

#include "utils/Logger.h"
#include "utils/Parser.h"
#include "cxl/CxlTypes.h"
#include "AddressList.h"
#include "Test.h"

int main(int argc, char** argv)
{
    // build singleton logger
    std::shared_ptr<Logger> logger = Logger::build();
    // build parser object
    std::shared_ptr<Parser> parser = std::make_shared<Parser>();
    // create new test
    std::shared_ptr<Test> test = std::make_shared<Test>();

    // force verbose level for max debug
    logger->verbose(0);
    // print welcome message, be nice :)
    logger->print("CXLStressTester version 0.1 (WM).", 201);

    // catch any hammer file regex exceptions
    try {
      // parse strings which command from command line when CXLStressTester is executed
      parser->parse_command_line(argc, argv);
      // parse test file, store information in data structs
      parser->parse_hammer_file(test->targets, test->threads_define);
    }
    catch (...) {
      logger->print("regex parsing error : Please fix the the hammer file", 200);
      return -1;
    }

    // at this point, all parsing went okay, now save data into thread data structs
    test->load_generators();

    // configure test case
    test->configure();

    // clean memory before starting test
    test->clear_memory();

    logger->print("Press any key to start generators.", 200);
    std::cin.get();

    test->start();

    logger->print("Running. Press enter to stop generators.", 200);
    std::cin.get();
    
    test->stop();

    bool result = test->verify();
    
    // change this to be inside the test
    if (parser->display_dump){ test->dump(); }

    // say good bye
    logger->print("Exiting.", 201);

    if (result) {
     return 0;
    }
    return -1;
}
