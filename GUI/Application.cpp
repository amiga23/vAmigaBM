// -----------------------------------------------------------------------------
// This file is part of vAmiga Bare Metal
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Application.h"
#include "Controller.h"
#include "Amiga.h"

Application::Application(int argc, const char *argv[]) :
GUIComponent(*this),
controller(*this),
inputManager(*this),
os(*this),
console(*this),
splashScreen(*this),
canvas(*this),
musicStream(*this)
{
    for (int i = 0; i < argc; i++) {
        this->argv.push_back(string(argv[i]));
    }
}

Application::~Application()
{
}

void
Application::check()
{
    printf("Application::check()\n");
    
    sf::Joystick::update();
    for (int i = 0; sf::Joystick::isConnected(i); i++) {
        printf("Joystick %d: ", i);
        printf("%d buttons ",  sf::Joystick::getButtonCount(i));
        printf("%s ", sf::Joystick::hasAxis(0, sf::Joystick::Axis::X) ? "X" : "x");
        printf("%s ", sf::Joystick::hasAxis(0, sf::Joystick::Axis::Y) ? "Y" : "y");
        printf("\n");
    }
        
    // Initialize ManyMouse
    const int available_mice = ManyMouse_Init();
    int i;

    if (available_mice < 0) {
        printf("Error initializing ManyMouse!\n");
        ManyMouse_Quit();
        return;
    }
    printf("ManyMouse driver: %s\n", ManyMouse_DriverName());

    if (available_mice == 0) {
        printf("No mice detected!\n");
        ManyMouse_Quit();
        return;
    }

    for (i = 0; i < available_mice; i++) {
        printf("#%d: %s\n", i, ManyMouse_DeviceName(i));
    }
    printf("\n");

    // Check for shader support
    if (!sf::Shader::isAvailable()) {
        throw Exception("No shader support");
    }
}

void
Application::init()
{
    // Setup window dimensions
    winXmin = OS::scale(canvas.textureRect.width);
    winYmin = OS::scale(canvas.textureRect.height);
    int winXinit = winXmin;
    int winYinit = winYmin;
    
    // Create render window
    auto videoMode = sf::VideoMode(winXinit, winYinit);
    window.create(videoMode, "vAmiga Bare Metal");

    if (!window.isOpen()) {
        throw Exception("Unable to create window");
    }
        
    controller.init();
    splashScreen.init();
    canvas.init();
    console.init();
    musicStream.init();
    
    // Fire a resize event
    resize(window.getSize().x, window.getSize().y);
}

void
Application::awake()
{
    controller.awake();
    splashScreen.awake();
    canvas.awake();
    console.awake();
}

void
Application::run()
{
    sf::Event event;
    sf::Clock clock;
    u64 frames = 0, latchedFrames = 0;
    float elapsedTime = 0;
    
    // Render at 60 Hz
    // window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    
    // Start music stream
    musicStream.setVolume(50.0); 
    musicStream.play();
    
    inputManager.connectMouse(0, PORT_1);
    inputManager.connectJoystick(0, PORT_2);
    inputManager.connectKeyset (0, PORT_2);

    while (window.isOpen()) {
        
        sf::Time dt = clock.restart();
        elapsedTime += dt.asSeconds();

        while (window.pollEvent(event)) respond(event);
        update(frames++, dt);
        render();
        
        // EXPERIMENTAL
        /*
        ManyMouseEvent mmevent;
        while (ManyMouse_PollEvent(&mmevent)) {
             if (mmevent.type == MANYMOUSE_EVENT_RELMOTION) {
                 printf("Mouse #%u relative motion %s %d\n", mmevent.device, mmevent.item == 0 ? "X" : "Y", mmevent.value);

                 sf::Vector2f vec;
                 if (mmevent.item == 0) {
                     vec.x += mmevent.value;
                 } else {
                     vec.y += mmevent.value;
                 }

             } else if (mmevent.type == MANYMOUSE_EVENT_ABSMOTION) { // I have never witnessed this event
                 printf("Mouse #%u absolute motion %s %d\n", mmevent.device, mmevent.item == 0 ? "X" : "Y", mmevent.value);
             } else if (mmevent.type == MANYMOUSE_EVENT_BUTTON) {
                 printf("Mouse #%u button %u %s\n", mmevent.device, mmevent.item, mmevent.value ? "down" : "up");
             } else if (mmevent.type == MANYMOUSE_EVENT_SCROLL) {
                 const char *wheel;
                 const char *direction;
                 if (mmevent.item == 0) {
                     wheel = "vertical";
                     direction = ((mmevent.value > 0) ? "up" : "down");
                 } else {
                     wheel = "horizontal";
                     direction = ((mmevent.value > 0) ? "right" : "left");
                 }
                 printf("Mouse #%u wheel %s %s\n", mmevent.device, wheel, direction);
             } else if (mmevent.type == MANYMOUSE_EVENT_DISCONNECT) {
                 printf("Mouse #%u disconnect\n", mmevent.device);
             } else {
                 printf("Mouse #%u unhandled event type %d\n", mmevent.device, mmevent.type);
             }
         }
        */
        
        // Compute the frames per second once in a while
        if (elapsedTime > 1.0) {
            fps((frames -latchedFrames) / elapsedTime);
            latchedFrames = frames;
            elapsedTime = 0;
        }
    }
    
    controller.deinit();
}

void
Application::respond(sf::Event &event)
{
    switch (event.type) {
            
        case sf::Event::Closed:
            window.close();
            break;
            
        case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::F11) console.toggle();
            break;
            
        case sf::Event::MouseButtonPressed:
            break;
            
        case sf::Event::Resized:
            resize(event.size.width, event.size.height);
            break;
            
        default:
            break;
    }
    
    // Distribute the event to the uppermost visible layer
    if (console.isVisible()) console.respond(event);
    else if (canvas.isVisible()) canvas.respond(event);
    else if (splashScreen.isVisible()) splashScreen.respond(event);
}

void
Application::resize(float w, float h)
{
    // Restore the minimal window size if the proposed size is smaller
    if (w < winXmin || h < winYmin) {
        
        w = winXmin;
        h = winYmin;
        
        window.setSize(sf::Vector2u{(unsigned)w,(unsigned)h});
    }
    
    // Adjust the view to the new size
    window.setView(sf::View(sf::FloatRect(0, 0, w, h)));

    // Inform all layers
    splashScreen.resize(w, h);
    canvas.resize(w, h);
    console.resize(w, h);
}

void
Application::update(u64 frames, sf::Time dt)
{
    inputManager.poll();

    splashScreen.update(frames, dt);
    canvas.update(frames, dt);
    console.update(frames, dt);
}

void
Application::render()
{
    window.clear();
    
    if (canvas.isTransparent()) splashScreen.render();
    if (canvas.isVisible()) canvas.render();
    if (console.isVisible()) console.render();

    window.display();
}

void
Application::fps(float fps)
{
    printf("fps: %.2f\n", fps);
}

void
Application::play(SoundID id, float volume, isize min, isize max)
{
    // Search the pool for a free sound object
    for (isize i = min; i <= max; i++) {

        if (sound[i].getStatus() == sf::SoundSource::Status::Stopped) {
            
            sound[i].setBuffer(assets.get(id));
            sound[i].setVolume(volume);
            sound[i].play();
            break;
        }
    }
}
