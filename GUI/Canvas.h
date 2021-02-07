// -----------------------------------------------------------------------------
// This file is part of vAmiga Bare Metal
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Application.h"

class Canvas : public Layer {
    
    // The current screen buffer
    ScreenBuffer screenBuffer = { nullptr, false };
    
    // The emulator texture
    sf::Texture emuTex;
 
    // Render object
    sf::RectangleShape foreground;
    
    
    //
    // Initializing
    //
    
public:
    
    Canvas(class Application &ref);
    ~Canvas();
    
    bool init();
    
    
    //
    // Methods from Layer class
    //
    
    void handle(const sf::Event &event) override;
    void update(sf::Time dt) override; 
    void render() override;
    
    
    //
    // Opening and closing
    //
    
public:
    
    // Shows or hides the canvas window
    void open() { setTargetAlpha(0xFF, 0.5); }
    void close() { setTargetAlpha(0x00, 0.5); }
    void toggle() { isVisible() ? close() : open(); }
};
