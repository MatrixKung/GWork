
#include <windows.h>
#include <gdiplus.h>

#include "Gwork/Renderers/GDIPlus.h"
#include "Gwork/Utility.h"
#include "Gwork/Font.h"
#include "Gwork/Texture.h"
#include "Gwork/WindowProvider.h"

#include <math.h>

namespace Gwk
{
    namespace Renderer
    {
        GDIPlus::GDIPlus(HWND pHWND)
        {
            m_HWND = pHWND;
            m_hDC = NULL;
            graphics = NULL;
            m_iWidth = 0;
            m_iHeight = 0;
            // Initialize GDI+.
            Gdiplus::GdiplusStartupInput gdiplusStartupInput;
            Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
        }

        GDIPlus::~GDIPlus()
        {
            Gdiplus::GdiplusShutdown(m_gdiplusToken);
        }

        void GDIPlus::Begin()
        {
            m_hDC = GetDC(m_HWND);
            graphics = Gdiplus::Graphics::FromHDC(m_hDC);
        }

        void GDIPlus::End()
        {
            if (graphics)
            {
                delete graphics;
                graphics = NULL;
            }

            ReleaseDC(m_HWND, m_hDC);
            m_hDC = NULL;
        }

        void GDIPlus::DrawFilledRect(Gwk::Rect rect)
        {
            Translate(rect);
            Gdiplus::SolidBrush solidBrush(m_Colour);
            graphics->FillRectangle(&solidBrush, rect.x, rect.y, rect.w, rect.h);
        }

        void GDIPlus::SetDrawColor(Gwk::Color color)
        {
            m_Colour = Gdiplus::Color(color.a, color.r, color.g, color.b);
        }

        void GDIPlus::LoadFont(Gwk::Font* font)
        {
            Gdiplus::FontStyle fs = Gdiplus::FontStyleRegular;
            font->realsize = font->size*Scale();
            Gdiplus::Font* pFont = new Gdiplus::Font(Utility::Widen(font->facename).c_str(),
                                                     font->realsize, fs, Gdiplus::UnitPixel, NULL);
            font->data = pFont;
        }

        void GDIPlus::FreeFont(Gwk::Font* pFont)
        {
            if (!pFont->data)
                return;

            Gdiplus::Font* font = ((Gdiplus::Font*)pFont->data);
            delete font;
            pFont->data = NULL;
        }

        void GDIPlus::RenderText(Gwk::Font* pFont, Gwk::Point pos,
                                 const Gwk::String& text)
        {
            Translate(pos.x, pos.y);

            // If the font doesn't exist, or the font size should be changed
            if (!pFont->data || fabs(pFont->realsize-pFont->size*Scale()) > 2)
            {
                FreeFont(pFont);
                LoadFont(pFont);
            }

            Gdiplus::StringFormat strFormat(Gdiplus::StringFormat::GenericDefault());
            Gdiplus::SolidBrush solidBrush(m_Colour);
            Gdiplus::RectF r(pos.x, pos.y, 1000, 1000);
            Gdiplus::Font* pGDIFont = (Gdiplus::Font*)pFont->data;
            const std::wstring wtext( Utility::Widen(text) );
            graphics->DrawString(wtext.c_str(), wtext.length()+1,
                                 pGDIFont, r, &strFormat, &solidBrush);
        }

        Gwk::Point GDIPlus::MeasureText(Gwk::Font* pFont, const Gwk::String& text)
        {
            Gwk::Point p(1, 1);

            if (!pFont->data || fabs(pFont->realsize-pFont->size*Scale()) > 2)
            {
                FreeFont(pFont);
                LoadFont(pFont);
            }

            Gdiplus::StringFormat strFormat(Gdiplus::StringFormat::GenericDefault());
            strFormat.SetFormatFlags(
                Gdiplus::StringFormatFlagsMeasureTrailingSpaces|strFormat.GetFormatFlags());
            Gdiplus::SizeF size;
            Gdiplus::Graphics g(m_HWND);
            Gdiplus::Font* pGDIFont = (Gdiplus::Font*)pFont->data;
            const std::wstring wtext( Utility::Widen(text) );
            g.MeasureString(wtext.c_str(), -1, pGDIFont, Gdiplus::SizeF(10000, 10000),
                            &strFormat, &size);
            return Gwk::Point(size.Width+1, size.Height+1);
        }

        void GDIPlus::StartClip()
        {
            const Gwk::Rect& rect = ClipRegion();
            graphics->SetClip(Gdiplus::Rect(rect.x*Scale(), rect.y*Scale(), rect.w*Scale(), rect.h*
                                            Scale()), Gdiplus::CombineModeReplace);
        }

        void GDIPlus::EndClip()
        {
            graphics->ResetClip();
        }

        void GDIPlus::DrawTexturedRect(Gwk::Texture* pTexture, Gwk::Rect pTargetRect, float u1,
                                       float v1, float u2, float v2)
        {
            Gdiplus::Bitmap* pImage = (Gdiplus::Bitmap*)pTexture->data;

            // Missing image, not loaded properly?
            if (!pImage || pImage->GetType() == Gdiplus::ImageTypeUnknown)
                return DrawMissingImage(pTargetRect);

            Translate(pTargetRect);
            Gdiplus::RectF TargetRect(pTargetRect.x, pTargetRect.y, pTargetRect.w, pTargetRect.h);
            // Convert UV to pixel coords
            float fW = pImage->GetWidth();
            float fH = pImage->GetHeight();
            u1 *= fW;
            v1 *= fH;
            u2 *= fW;
            u2 -= u1;
            v2 *= fH;
            v2 -= v1;
            graphics->DrawImage(pImage, TargetRect, u1, v1, u2, v2, Gdiplus::UnitPixel);
        }

        void GDIPlus::LoadTexture(Gwk::Texture* pTexture)
        {
            Gdiplus::Bitmap* pImage = new Gdiplus::Bitmap(Utility::Widen(pTexture->name).c_str());
            pTexture->data = pImage;
            pTexture->width = pImage->GetWidth();
            pTexture->height = pImage->GetHeight();
        }

        void GDIPlus::FreeTexture(Gwk::Texture* pTexture)
        {
            Gdiplus::Bitmap* pImage = (Gdiplus::Bitmap*)pTexture->data;

            if (!pImage)
                return;

            delete pImage;
        }

        Gwk::Color GDIPlus::PixelColour(Gwk::Texture* pTexture, unsigned int x, unsigned int y,
                                         const Gwk::Color& col_default)
        {
            Gdiplus::Bitmap* pImage = (Gdiplus::Bitmap*)pTexture->data;

            if (!pImage)
                return col_default;

            Gdiplus::Color c;
            pImage->GetPixel(x, y, &c);
            return Gwk::Color(c.GetR(), c.GetG(), c.GetB(), c.GetA());
        }

        bool GDIPlus::InitializeContext(Gwk::WindowProvider* pWindow)
        {
            m_HWND = (HWND)pWindow->GetWindow();
            return true;
        }

        bool GDIPlus::ShutdownContext(Gwk::WindowProvider* pWindow)
        {
            return true;
        }

        bool GDIPlus::PresentContext(Gwk::WindowProvider* pWindow)
        {
            return true;
        }

        bool GDIPlus::ResizedContext(Gwk::WindowProvider* pWindow, int w, int h)
        {
            return true;
        }

        bool GDIPlus::BeginContext(Gwk::WindowProvider* pWindow)
        {
            return true;
        }

        bool GDIPlus::EndContext(Gwk::WindowProvider* pWindow)
        {
            return true;
        }

    }
}