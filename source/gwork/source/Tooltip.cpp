/*
 *  Gwork
 *  Copyright (c) 2010 Facepunch Studios
 *  Copyright (c) 2013-17 Nick Trout
 *  See license in Gwork.h
 */


#include <Gwork/Tooltip.h>
#include <Gwork/Utility.h>
#include <Gwork/InputHandler.h>
#include <Gwork/Controls/Canvas.h>

namespace Gwk
{
namespace Tooltip
{

static Controls::Base* g_toolTip = nullptr;

bool TooltipActive()
{
    return g_toolTip != nullptr;
}

void Enable(Controls::Base* control)
{
    if (!control->GetTooltip())
        return;

    g_toolTip = control;
}

void Disable(Controls::Base* control)
{
    if (g_toolTip == control)
        g_toolTip = nullptr;
}

static Gwk::Rect ClampRectToRect(Gwk::Rect inside, Gwk::Rect const& outside, bool clampSize)
{
    if (inside.x < outside.x)
        inside.x = outside.x;

    if (inside.y  < outside.y)
        inside.y = outside.y;

    if (inside.x+inside.w > outside.x+outside.w)
    {
        if (clampSize)
            inside.w = outside.w;
        else
            inside.x = outside.x+outside.w-inside.w;
    }

    if (inside.y+inside.h > outside.y+outside.h)
    {
        if (clampSize)
            inside.h = outside.h;
        else
            inside.y = outside.w+outside.h-inside.h;
    }

    return inside;
}

void RenderTooltip(Skin::Base* skin)
{
    if (!g_toolTip)
        return;

    Gwk::Renderer::Base* render = skin->GetRender();
    Gwk::Point oldRenderOffset = render->GetRenderOffset();
    Gwk::Point MousePos = Input::GetMousePosition();
    Gwk::Rect Bounds = g_toolTip->GetTooltip()->GetBounds();
    Gwk::Rect rOffset = Gwk::Rect(MousePos.x - Bounds.w*0.5f, MousePos.y-Bounds.h-10,
                                  Bounds.w, Bounds.h);
    rOffset = ClampRectToRect(rOffset, g_toolTip->GetCanvas()->GetBounds(), false);

    // Calculate offset on screen bounds
    render->AddRenderOffset(rOffset);
    render->EndClip();
    skin->DrawTooltip(g_toolTip->GetTooltip());
    g_toolTip->GetTooltip()->DoRender(skin);
    render->SetRenderOffset(oldRenderOffset);
}

void ControlDeleted(Controls::Base* control)
{
    Disable(control);
}

} // namespace Tooltip
} // namespace Gwk