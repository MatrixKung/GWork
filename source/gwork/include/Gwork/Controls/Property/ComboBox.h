/*
 *  Gwork
 *  Copyright (c) 2012 Facepunch Studios
 *  Copyright (c) 2015-2016 Billy Quith
 *  See license in Gwork.h
 */

#pragma once
#ifndef GWK_CONTROLS_PROPERTY_COMBOBOX_H
#define GWK_CONTROLS_PROPERTY_COMBOBOX_H

#include "Gwork/Controls/Property/BaseProperty.h"
#include "Gwork/Controls/ComboBox.h"

namespace Gwk
{
    namespace Controls
    {
        namespace Property
        {
            class GWK_EXPORT ComboBox : public Property::Base
            {
            public:

                GWK_CONTROL_INLINE(ComboBox, Property::Base)
                {
                    m_ComboBox = new Gwk::Controls::ComboBox(this);
                    m_ComboBox->Dock(Docking::Fill);
                    m_ComboBox->onSelection.Add(this, &ParentClass::OnPropertyValueChanged);
                    m_ComboBox->SetTabable(true);
                    m_ComboBox->SetKeyboardInputEnabled(true);
                    m_ComboBox->SetShouldDrawBackground(false);
                    SetHeight(18);
                }

                virtual String GetPropertyValue() override
                {
                    Gwk::Controls::Label* pControl = m_ComboBox->GetSelectedItem();

                    if (!pControl)
                        return "";

                    return pControl->GetName();
                }

                virtual void SetPropertyValue(const String& val, bool bFireChangeEvents) override
                {
                    m_ComboBox->SelectItemByName(val, bFireChangeEvents);
                }

                virtual bool IsEditing() override
                {
                    return m_ComboBox->HasFocus();
                }

                virtual bool IsHovered() override
                {
                    return ParentClass::IsHovered() || m_ComboBox->IsHovered();
                }

                Gwk::Controls::ComboBox* GetComboBox()
                {
                    return m_ComboBox;
                }

            protected:

                Gwk::Controls::ComboBox* m_ComboBox;
            };


        }
    }
}
#endif // ifndef GWK_CONTROLS_PROPERTY_COMBOBOX_H