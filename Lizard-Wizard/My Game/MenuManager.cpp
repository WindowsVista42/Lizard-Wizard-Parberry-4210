#include "Game.h"


void CGame::InitializeMenu(){

    Entity e = Entity();

    auto getSpriteInstance = [&]() {

        SpriteInstance menu_Sprite;

        menu_Sprite.position = Vec2(100.0f, 100.0f);
        menu_Sprite.roll = 0.0f;
        menu_Sprite.scale = Vec2(100.0f, 100.0f);
        menu_Sprite.rgba = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        menu_Sprite.texture_index = TextureIndex::White;
        
        return menu_Sprite;

    };
    

    {

        SpriteInstance MAIN = getSpriteInstance();
    
        m_Menu.AddExisting(e, MAIN);
        m_MainMenu.AddExisting(e);

       

        Panel panel;

        panel.Hovering = false; // GetCursorPos(); x+scal & y+scale 
        panel.Pressed = false; // m_leftclick.pressed();
        panel.Tint = Vec3(0.4f);
        panel.nextState = MenuState::MAIN;

        m_Panel.AddExisting(e, panel);


        PanelText panelText;

        panelText.Text;
        panelText.position = Vector2(0.0f, 0.0f);
        panelText.color = Vec4(0.0f, 0.0f, 0.0f, 1.0f);


        m_PanelText.AddExisting(e, panelText);

    }
    
    
}

void CGame::InputMenu() {
    
    POINT cursorPos;
    POINT temp;
    RECT rect;
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

        if (
            cursorPos.x   > position.x && cursorPos.y > position.y && 
            cursorPos.x < position.x + sprite->scale.x && cursorPos.y < position.y + sprite->scale.y  
            ) {
            panel->Hovering = true; 
            panel->Tint = 0.5f;
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
        
    });
    
    
}

void CGame::RenderMenu() {

    

}

















