#define _CRT_SECURE_NO_WARNINGS
#include "App.h"

#include "Textures.h"
#include "Render.h"
#include "ModuleFonts.h"

#include <string.h>

ModuleFonts::ModuleFonts() : Module() {}

ModuleFonts::~ModuleFonts() {}

// Load new texture from file path
int ModuleFonts::Load(const char* texture_path, const char* characters, uint rows) {
    int id = -1;

    if (texture_path == nullptr || characters == nullptr || rows == 0) {
        return id;
    }

	SDL_Texture* tex = app->tex->Load(texture_path);
    if (tex == nullptr || strlen(characters) >= MAX_FONT_CHARS) {
        return id;
    }

    id = 0;
    for (; id < MAX_FONTS; ++id) { if (fonts[id].texture == nullptr) { break; } }

    if (id == MAX_FONTS) {
        return id;
    }

    Font& font = fonts[id];

    font.texture = tex;
    font.rows = rows;

    // totalLength ---	length of the lookup table
    // table ---------  All characters displayed in the same order as the texture
    // columns -------  Amount of chars per row of the texture
    // char_w --------	Width of each character
    // char_h --------	Height of each character
    font.totalLength = strlen(characters);
    strcpy(font.table, characters);
    font.columns = (font.totalLength / rows);
    app->tex->GetSize(font.texture, font.char_w, font.char_h);
    font.char_w = font.char_w / font.columns;
    font.char_h = font.char_h / font.rows;

    return id;
}

void ModuleFonts::Unload(int font_id) {
    if (font_id >= 0 && font_id < MAX_FONTS && fonts[font_id].texture != nullptr) {
		app->tex->UnLoad(fonts[font_id].texture);
        fonts[font_id].texture = nullptr;
    }
}

void ModuleFonts::BlitText(int x, int y, int font_id, const char* text) const {
    if (text == nullptr || font_id < 0 || font_id >= MAX_FONTS || fonts[font_id].texture == nullptr) {
        return;
    }

    const Font* font = &fonts[font_id];
    SDL_Rect spriteRect;
	uint len = strlen(text);
    spriteRect.w = font->char_w;
    spriteRect.h = font->char_h;


    for (uint i = 0; i < len; ++i) {
        // Find the location of the current character in the lookup table
        for (uint j = 0; j < font->totalLength; ++j) {
            if (font->table[j] == text[i]) {
                // Retrieve the position of the current character in the sprite
                spriteRect.x = (j % font->columns) * spriteRect.w;
                spriteRect.y = (j / font->columns) * spriteRect.h;
                // Blit the character at its proper position
				app->render->DrawTexture(font->texture, x, y, &spriteRect);
                break;
            }
        }
        // Advance the position where we blit the next character
        x += spriteRect.w;
    }
}