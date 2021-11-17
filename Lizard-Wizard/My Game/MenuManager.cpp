#include "Game.h"


void CGame::InitializeMenu(){

    Entity e = Entity();


    auto getSpriteDesc = [&](Vector2 Pos, UINT Index, f32 XScale, f32 YScale) {

        LSpriteDesc2D menu;

        menu.m_vPos = Pos; //
        menu.m_nSpriteIndex = Index; //
        menu.m_nCurrentFrame = 0;
        menu.m_fXScale = XScale; //
        menu.m_fYScale = YScale; //
        menu.m_fRoll = 0.0f;
        menu.m_f4Tint = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        menu.m_fAlpha = 1.0f;

        
        return menu;

    };


    {

        LSpriteDesc2D MAIN = getSpriteDesc(Vector2 (2,1) , TextureIndex::White, 100.0f, 100.0f);

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

  //  GetClientRect(m_pRenderer->GetHwnd(), &rect);
 //   ClientToScreen(m_pRenderer->GetHwnd(), &rect);
    GetCursorPos(&cursorPos);
    ClientToScreen(m_pRenderer->GetHwnd(), &temp);

    cursorPos.x -= temp.x;
    cursorPos.y -= temp.y;

    std::cout << "X: " << cursorPos.x << "   Y: " << cursorPos.y << std::endl;

    Ecs::ApplyEvery(m_MainMenu, [=](Entity e) {

        LSpriteDesc2D* sprite = m_Menu.Get(e);
        Panel* panel = m_Panel.Get(e);

        Vector2 position = sprite->m_vPos;
        

        //std::cout << "X: " << position.x <<"   Y: " << position.y << std::endl;

        if (
            cursorPos.x   > position.x && cursorPos.y > position.y && 
            cursorPos.x < position.x + sprite->m_fXScale && cursorPos.y < position.y + sprite->m_fYScale   
            ) {
            panel->Hovering = true; std::cout << "Hovering" << std::endl;
        }
        else { panel->Hovering = false; std::cout << "Not Hovering" << std::endl; }

        if (pressed == true)
        {
            panel->Pressed = true; 
            std::cout << "Panel Pressed" << std::endl;
        }
        else
        {
            panel->Pressed = false;
            std::cout << "Panel NOT Pressed" << std::endl;
        }


        if (held == true)
        {
            panel->Pressed = true;
            std::cout << "Panel Held" << std::endl;
        }
        else
        {
            panel->Pressed = false;
            std::cout << "Panel NOT Held" << std::endl;
        }

    });



}

void CGame::RenderMenu() {

    

}

















