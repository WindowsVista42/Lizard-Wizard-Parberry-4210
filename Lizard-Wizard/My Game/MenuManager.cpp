#include "Game.h"

//Note: try to add images next to the menu text prompts play button, cog, and X


void CGame::InitializeMenu(){

    Entity e = Entity();
    auto getSpriteInstance = [&]() {

        SpriteInstance menu_Sprite;

        menu_Sprite.position = Vec2(100.0f, 100.0f); // 100.0f
        menu_Sprite.roll = 0.0f;
        menu_Sprite.scale = Vec2(100.0f, 100.0f);
        menu_Sprite.rgba = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        menu_Sprite.texture_index = TextureIndex::White;
        
        return menu_Sprite;

    };

    auto getPanelInstance = [&]() {

        Panel panel_Sprite;

        panel_Sprite.Hovering = false;
        panel_Sprite.Pressed = false;
        panel_Sprite.Tint = Vec3(0.4f);
        panel_Sprite.Position = Vec2(0.0f);
        panel_Sprite.nextState = MenuState::MAIN;

        return panel_Sprite;

    };
    


    {

        SpriteInstance MAIN = getSpriteInstance();
        m_Menu.AddExisting(e, MAIN);
        

        Panel PANEL = getPanelInstance();
        m_Panel.AddExisting(e, PANEL);
        
        m_MainMenu.AddExisting(e);
/*
        Panel panel;

        panel.Hovering = false; // GetCursorPos(); x+scal & y+scale 
        panel.Pressed = false; // m_leftclick.pressed();
        panel.Tint = Vec3(0.4f);
        panel.Position = Vec2(0.0f);
        panel.nextState = MenuState::MAIN;

        m_Panel.AddExisting(e, panel);
        */
      //  m_Panel.AddExisting(e, panel);


        PanelText panelText;

        panelText.Text;
        panelText.position = Vector2(0.0f, 0.0f);
        panelText.color = Vec4(0.0f, 0.0f, 0.0f, 1.0f);


        m_PanelText.AddExisting(e, panelText);

    }
    
    
}

void CGame::MenuInput() {
    
    POINT cursorPos;
    POINT temp;
    bool pressed;
    bool held;

    pressed = m_leftClick.pressed;
    held = m_leftClick.held;

    temp.x = 0;
    temp.y = 0;

    GetCursorPos(&cursorPos);
    ClientToScreen(m_pRenderer->GetHwnd(), &temp);

    cursorPos.x -= temp.x;
    cursorPos.y -= temp.y;

    //std::cout << "X: " << cursorPos.x << "   Y: " << cursorPos.y << std::endl;

    
    Ecs::ApplyEvery(m_MainMenu, [=](Entity e) {

        SpriteInstance* sprite = m_Menu.Get(e);
        Panel* panel = m_Panel.Get(e);

        Vector2 position = sprite->position;
        panel->Position.x = position.x;
        panel->Position.y = position.y;

        if (
            cursorPos.x > position.x && 
            cursorPos.y > position.y && 
            cursorPos.x < position.x + sprite->scale.x && 
            cursorPos.y < position.y + sprite->scale.y  
            ) {
            panel->Hovering = true; 
            panel->Tint = 0.5f;
            //panel->Position = position;
            //std::cout << "Hovering" << std::endl;

            if (panel->Hovering == true && pressed == true)
            {
                panel->Pressed = true;
                std::cout << "Panel Pressed" << std::endl;
            }

        }
        else { 
            panel->Hovering = false; 
            panel->Pressed = false; 
        }

        /*
        if (held == true)
        {
            panel->Pressed = true;
            //std::cout << "Panel Held" << std::endl;
        }
        else
        {
            panel->Pressed = false;
            //std::cout << "Panel NOT Held" << std::endl;
        }
        */

        /*// SETTINGS BUTTON
        * if(panel->Hovering == true && pressed == true)
        * {
        *       panel->Pressed = true;
        *       panel.nextState = MenuState::SETTINGS;
        *       std::cout << "Panel Pressed" << std::endl;
        *
        * }
        * if(same conditions for EXIT BUTTON)
        * {
        *   panel.nextState = MenuState::EXIT;
        *   Restart Game
        * }
        *   
        */
        
    });
    
    
}

void CGame::RenderMenu() {

    // MAIN MENU
    {
        m_pRenderer->BeginUIDrawing();

        char buffer[64];

        if (m_DrawMainMenu == true) {

            Panel panel;
            PanelText panel_text;
            SpriteInstance sprite_instance;
          
            
            // Background
            sprite_instance.position = Vec2(100.0f, 100.0f);
            sprite_instance.roll = 0.0f;
            sprite_instance.scale = Vec2(800.0f, 600.0f);
            sprite_instance.rgba = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
            sprite_instance.texture_index = TextureIndex::White;
            m_pRenderer->DrawSpriteInstance(&sprite_instance);
            
          
            
            
            // Resume Button -- Green
            sprite_instance.position = Vec2(100.0f, 100.0f);
            sprite_instance.roll = 0.0f;
            sprite_instance.scale = Vec2(800.0f, 200.0f);
            sprite_instance.rgba = Vec4(0.0f, 1.0f, 0.0f, 1.0f);
            sprite_instance.texture_index = TextureIndex::White;
            m_pRenderer->DrawSpriteInstance(&sprite_instance);     
            
            
            sprintf(buffer, "RESUME");
            m_pRenderer->DrawScreenText(buffer, Vec2(400.0f, 150.0f),Colors::White);
            panel.nextState = MenuState::RESUME;
            
            

            // Settings Button -- Black
            sprite_instance.position = Vec2(100.0f, 300.0f);
            sprite_instance.roll = 0.0f;
            sprite_instance.scale = Vec2(800.0f, 200.0f);
            sprite_instance.rgba = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
            sprite_instance.texture_index = TextureIndex::White;
            m_pRenderer->DrawSpriteInstance(&sprite_instance);

            sprintf(buffer, "SETTINGS");
            m_pRenderer->DrawScreenText(buffer, Vec2(450.0f, 350.0f), Colors::White);

            panel.nextState = MenuState::SETTINGS;
 

            // Exit Button -- Red
            sprite_instance.position = Vec2(100.0f, 500.0f);
            sprite_instance.roll = 0.0f;
            sprite_instance.scale = Vec2(800.0f, 200.0f);
            sprite_instance.rgba = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
            sprite_instance.texture_index = TextureIndex::White;
            m_pRenderer->DrawSpriteInstance(&sprite_instance);

            sprintf(buffer, "EXIT");
            m_pRenderer->DrawScreenText(buffer, Vec2(450.0f, 600.0f), Colors::White);

            panel.nextState = MenuState::EXIT;

        }


        m_pRenderer->EndUIDrawing();
    }

}

















