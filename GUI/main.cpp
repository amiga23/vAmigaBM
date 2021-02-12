// -----------------------------------------------------------------------------
// This file is part of vAmiga Bare Metal
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Application.h"

int main(int argc, const char *argv[]) {

    Application app(argc, argv);
    
    // Launch the app
    try {
        app.check();
        app.init();
        app.configure();
        
        // app.configure(argc, argv);
        
    } catch (Exception &e) {
        std::cout << "vAmiga is not able to launch." << std::endl;
        std::cout << "Error: " << e.what() << std::endl;
        exit(1);
    }
    
    // Enter the event loop
    app.run();
    return 0;
}
