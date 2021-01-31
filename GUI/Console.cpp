// -----------------------------------------------------------------------------
// This file is part of vAmiga Bare Metal
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Application.h"

const std::string& Console::prompt = string("vAmiga\% ");

Console::Console(Application &ref) : application(ref)
{
}

Console::~Console()
{
}

bool
Console::init()
{
    sf::Font& font = Assets::get(FontID::console);

    // Initialize the text storage
    storage.push_back("");

    // Initialize the input buffer
    input.push_back("");

    *this << "Retro shell 0.1, Dirk W. Hoffmann, 2021." << '\n';
    *this << "Linked to vAmiga core " << V_MAJOR << '.' << V_MINOR << '.' << V_SUBMINOR;
    *this << ", built on " __DATE__ << " " << __TIME__ << '.' << '\n' << '\n';
    *this << "Type 'help' for a list of available commands." << '\n' << '\n';
    
    *this << prompt;
        
    // Initialize the cursor
    glyphWidth = font.getGlyph(32, fontSize, false).advance;
    cursor.setSize(sf::Vector2f(glyphWidth + 2, fontSize + 3));
    cursor.setFillColor(sf::Color(0xFF,0xFF,0xFF,0x80));

    // Initialize the render target
    if (!texture.create(width, height)) {
        throw std::runtime_error("Console: Can't allocate render texture");
    }
    drawable.setTextureRect(sf::IntRect(0, height, width, -height));
    drawable.setTexture(&texture.getTexture());
            
    // Initialize render items
    for (int i = 0; i < numRows; i++) {
        
        row[i].setFont(font);
        row[i].setString("");
        row[i].setCharacterSize(fontSize);
        row[i].setFillColor(sf::Color::White);
        row[i].setPosition(hposForCol(0), vposForRow(i));
    }
    
    return true;
}

Console&
Console::operator<<(char value)
{
    if (value == '\n') {

        // Newline (appends an empty line)
        storage.push_back("");

    } else if (value == '\r') {

        // Carriage return (clears the current line)
        storage.back() = "";
        
    } else {
        
        // Add a single character
        if (storage.back().length() >= numCols) storage.push_back("");
        storage.back() += value;
    }
    
    // Shorten the text storage if it has grown too large
    if (storage.size() > 100) storage.erase(storage.begin());
        
    return *this;
}

Console&
Console::operator<<(const std::string& text)
{
    size_t remaining = numCols - storage.back().length();
    
    // Split the string if it is too large
    if (text.length() > remaining) {
        *this << text.substr(0, remaining) << '\n' << text.substr(remaining);
    } else {
        storage.back() += text;
    }
    
    // Shorten the text storage if it has grown too large
    if (storage.size() > 100) storage.erase(storage.begin());

    return *this;
}

Console&
Console::operator<<(int value)
{
    *this << std::to_string(value);
    return *this;
}

void
Console::tab(int hpos)
{
    int delta = hpos - (int)storage.back().length();
    for (int i = 0; i < delta; i++) {
        *this << ' ';
    }
}


void
Console::replace(const string& text, const string& prefix)
{
    
    storage.back() = prefix + text.substr(0, numCols);
}

void
Console::list()
{
    printf("<------\n");
    for (auto &it : storage) {
        printf("%s\n", it.c_str());
    }
    printf("------>\n");
}

void
Console::scrollToLine(int line)
{
    line = std::max(line, 0);
    line = std::min(line, (int)storage.size() - 1);
        
    isDirty = line != first;
    first = line;
}

void
Console::scrollToTop()
{
    scrollToLine(0);
}

void
Console::scrollToBottom()
{
    scrollToLine((int)storage.size() - numRows);
}

int
Console::rowOfLastLine()
{
    return (int)storage.size() - first - 1;
}

void
Console::type(char c)
{    
    switch (c) {
            
        case '\n':

            *this << '\n';
            
            // Execute the command
            application.interpreter.exec(input[index]);
            
            // Add a new entry to the input buffer
            input.push_back("");
            index = (int)input.size() - 1;
            hpos = 0;
            
            // Print a new prompt
            *this << string(prompt);
            break;
            
        case '\b':
            
            if (hpos > 0) {
                input[index].erase(input[index].begin() + --hpos);
            }
            *this << '\r' << string(prompt) << input[index];
            break;
            
        default:
            
            if (input[index].length() < numCols - (int)prompt.length() - 1) {
                
                input[index].insert(input[index].begin() + hpos++, c);
            }

            *this << '\r' << string(prompt) << input[index];
    }

    scrollToBottom();
    isDirty = true;
}

void
Console::keyPressed(const sf::Keyboard::Key& key)
{
    switch (key) {
            
        case sf::Keyboard::Up:

            printf("Cursor up %d\n", index);
            if (index > 0) {
                index--;
                hpos = (int)input[index].size();

                replace(input[index]);
            }
            break;

        case sf::Keyboard::Down:

            printf("Cursor down %d\n", index);
            if (index < input.size() - 1) {
                index++;
                hpos = (int)input[index].size();
                
                replace(input[index]);
            }
            break;
            
        case sf::Keyboard::Left:

            printf("Cursor left\n");
            if (hpos > 0) {
                hpos--;
            }
            printf("hpos = %d\n", hpos);
            break;
            
        case sf::Keyboard::Right:
            
            printf("Cursor right\n");
            if (hpos < input[index].length()) {
                hpos++;
            }
            printf("hpos = %d\n", hpos);
            break;
            
        case sf::Keyboard::Home:
            
            hpos = 0;
            break;

        case sf::Keyboard::End:
            
            hpos = (int)input[index].length();
            break;

        default:
            return;
    }
    
    scrollToBottom();
    isDirty = true;
}

void
Console::keyReleased(const sf::Keyboard::Key& key)
{
    /*
    printf("keyReleased\n");

    switch (key) {
            
        case sf::Keyboard::LControl:
            break;

        default:
            break;
    }
    */
}
            
void
Console::render(sf::RenderWindow &window)
{
    if (isAnimating()) {
        if (isOpening()) alpha = std::min(targetAlpha, alpha + 24);
        if (isClosing()) alpha = std::max(targetAlpha, alpha - 24);
        // drawable.setFillColor(sf::Color(0xFF,0xFF,0xFF,alpha));
        drawable.setFillColor(sf::Color(0xFF,0xFF,0xFF,alpha));
    }
    
    // TODO: MOVE TO A resize() function
    auto winSize = window.getSize();
    sf::Vector2f size = { (float)winSize.x, (float)winSize.y };
    drawable.setSize(size);
    
    if (isVisible()) {
        
        if (isDirty) {
            updateTexture();
            isDirty = false;
        }
        window.draw(drawable);
    }
}

void
Console::updateTexture()
{
    texture.clear(sf::Color(0x21,0x21,0x21,0xD0));
    
    for (int i = 0; i < numRows; i++) {
        
        if (first + i < storage.size()) {
            row[i].setString(storage[first + i]);
        } else {
            row[i].setString(""); // std::to_string(first + i));
        }
        texture.draw(row[i]);
    }
    
    // Draw cursor
    int cursorX = hposForCol(hpos + (int)prompt.length());
    int cursorY = vposForRow(rowOfLastLine()) + 3;
    cursor.setPosition(cursorX, cursorY);
    texture.draw(cursor);
}
