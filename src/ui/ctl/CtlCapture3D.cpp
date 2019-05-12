/*
 * CtlCapture3D.cpp
 *
 *  Created on: 12 мая 2019 г.
 *      Author: sadko
 */

#include <ui/ui.h>
#include <plugins/room_builder.h>

namespace lsp
{
    namespace ctl
    {
        
        void CtlCapture3D::LSPCaptureColor::color_changed()
        {
            LSPCapture3D *cap = widget_cast<LSPCapture3D>(pCapture->pWidget);
            if (cap == NULL)
                return;

            LSPColor c;
            c.copy(color());
            cap->color()->copy(&c);

            c.hue(fmodf(c.hue() + pCapture->fHueShift, 1.0f));
            cap->axis_color()->copy(&c);
        }

        CtlCapture3D::CtlCapture3D(CtlRegistry *src, LSPCapture3D *widget):
            CtlWidget(src, widget),
            sXColor(this)
        {
            fHueShift       = 1.0f/3.0f;
            sXColor.set_rgb(1.0f, 0.0f, 0.0f);

            dsp::init_point_xyz(&sCapture.sPos, 0.0f, 0.0f, 0.0f);
            sCapture.fYaw       = 0.0f;
            sCapture.fPitch     = 0.0f;
            sCapture.fRoll      = 0.0f;
            sCapture.fCapsule   = 0.015f;
            sCapture.sConfig    = RT_CC_MONO;
            sCapture.fAngle     = 0.0f;
            sCapture.fDistance  = 0.0f;
            sCapture.enDirection= RT_AC_EIGHT;
            sCapture.enSide     = RT_AC_EIGHT;

            pPosX           = NULL;
            pPosY           = NULL;
            pPosZ           = NULL;
            pYaw            = NULL;
            pPitch          = NULL;
            pRoll           = NULL;
            pSize           = NULL;
            pMode           = NULL;
            pAngle          = NULL;
            pDistance       = NULL;
        }
        
        CtlCapture3D::~CtlCapture3D()
        {
        }

        void CtlCapture3D::init()
        {
            sColor.init_hsl2(pRegistry, pWidget, &sXColor, A_COLOR, A_HUE_ID, A_SAT_ID, A_LIGHT_ID);
        }

        void CtlCapture3D::set(widget_attribute_t att, const char *value)
        {
            switch (att)
            {
                case A_XPOS_ID:
                    BIND_PORT(pRegistry, pPosX, value);
                    break;
                case A_YPOS_ID:
                    BIND_PORT(pRegistry, pPosY, value);
                    break;
                case A_ZPOS_ID:
                    BIND_PORT(pRegistry, pPosZ, value);
                    break;
                case A_YAW_ID:
                    BIND_PORT(pRegistry, pYaw, value);
                    break;
                case A_PITCH_ID:
                    BIND_PORT(pRegistry, pPitch, value);
                    break;
                case A_ROLL_ID:
                    BIND_PORT(pRegistry, pRoll, value);
                    break;
                case A_SIZE_ID:
                    BIND_PORT(pRegistry, pSize, value);
                    break;
                case A_MODE_ID:
                    BIND_PORT(pRegistry, pMode, value);
                    break;
                case A_ANGLE_ID:
                    BIND_PORT(pRegistry, pAngle, value);
                    break;
                case A_DISTANCE_ID:
                    BIND_PORT(pRegistry, pDistance, value);
                    break;

                default:
                    bool set = sColor.set(att, value);
                    if (!set)
                        CtlWidget::set(att, value);
                    break;
            }
        }

        void CtlCapture3D::notify(CtlPort *port)
        {
            CtlWidget::notify(port);

            bool sync = false;
            if (port == pPosX)
            {
                sCapture.sPos.x     = port->get_value();
                sync    = true;
            }
            if (port == pPosY)
            {
                sCapture.sPos.y     = port->get_value();
                sync    = true;
            }
            if (port == pPosZ)
            {
                sCapture.sPos.z     = port->get_value();
                sync    = true;
            }
            if (port == pYaw)
            {
                sCapture.fYaw       = port->get_value();
                sync    = true;
            }
            if (port == pPitch)
            {
                sCapture.fPitch     = port->get_value();
                sync    = true;
            }
            if (port == pRoll)
            {
                sCapture.fRoll      = port->get_value();
                sync    = true;
            }
            if (port == pSize)
            {
                sCapture.fCapsule   = port->get_value() * 0.5f;
                sync    = true;
            }
            if (port == pMode)
            {
                sCapture.sConfig    = room_builder_base::decode_config(port->get_value());
                sync    = true;
            }
            if (port == pAngle)
            {
                sCapture.fAngle     = port->get_value();
                sync    = true;
            }
            if (port == pDistance)
            {
                sCapture.fDistance  = port->get_value();
                sync    = true;
            }

            if (sync)
                sync_capture_state();
        }

        void CtlCapture3D::sync_capture_state()
        {
            // Get widget
            LSPCapture3D *cap = widget_cast<LSPCapture3D>(pWidget);
            if (cap == NULL)
                return;

            // Configure capture settings
            room_capture_settings_t cset;
            status_t res = room_builder_base::configure_capture(&cset, &sCapture);
            if (res != STATUS_OK)
                return;

            // Commit capture settings
            res = cap->set_items(2);
            if (res != STATUS_OK)
                return;

            cap->set_radius(sCapture.fCapsule);
            for (size_t i=0; i<2; ++i)
            {
                cap->set_position(i, &cset.pos[i].z);
                cap->set_direction(i, &cset.pos[i].v);
                cap->set_enabled(i, cset.n > i);
            }
        }
    
    } /* namespace ctl */
} /* namespace lsp */
