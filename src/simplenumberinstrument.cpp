/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  DashboardSK Plugin
 * Author:   Pavel Kalian
 *
 ******************************************************************************
 * This file is part of the DashboardSK plugin
 * (https://github.com/nohal/dashboardsk_pi).
 *   Copyright (C) 2022 by Pavel Kalian
 *   https://github.com/nohal
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3, or (at your option) any later
 * version of the license.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "simplenumberinstrument.h"
#include "dashboard.h"
#include "instrument.h"

PLUGIN_BEGIN_NAMESPACE

void SimpleNumberInstrument::Init()
{
    // Define formatting and transformation data to be shared between settings
    // GUI and instrument logic (We need it to be consistent)
#define X(a, b, c) m_supported_formats.Add(b);
    DSK_SNI_FORMATS
#undef X
#define X(a, b, c) m_format_strings.Add(c);
    DSK_SNI_FORMATS
#undef X
#define X(a, b) m_supported_transforms.Add(b);
    DSK_SNI_TRANSFORMATIONS
#undef X

    // Basic settings inherited from Instrument classs
    m_title = "???";
    m_name = _("New Simple Number");
    m_sk_key = wxEmptyString;
    // SimpleNumberInstrument's own settings
    m_timed_out = false;
    m_title_font = wxFont(
        15, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_body_font
        = wxFont(30, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

#define X(a, b, c, d, e, f, g, h) SetSetting(b, c);
    DSK_SNI_SETTINGS
#undef X

#define X(a, b, c, d, e, f, g, h)                                              \
    m_config_controls.emplace_back(                                            \
        config_control({ b, d, dskConfigCtrl::e, f }));
    DSK_SNI_SETTINGS
#undef X
};

void SimpleNumberInstrument::SetSetting(
    const wxString& key, const wxString& value)
{
    Instrument::SetSetting(key, value);
    if (key == "sk_key" && !m_sk_key.IsSameAs(value)) {
        m_sk_key = wxString(value);
        if (m_parent_dashboard) {
            m_parent_dashboard->Unsubscribe(this);
            m_parent_dashboard->Subscribe(m_sk_key, this);
        }
    } else if (key.IsSameAs("format") || key.IsSameAs("transformation")) {
        int i = 0;
        value.ToInt(&i);
        SetSetting(key, i);
    }
}

void SimpleNumberInstrument::SetSetting(const wxString& key, const int& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs("format")) {
        m_format_index = value;
    } else if (key.IsSameAs("transformation")) {
        m_transformation = static_cast<transformation>(value);
    }
}

wxBitmap SimpleNumberInstrument::Render(double scale)
{
    bool needs_redraw = false;
    wxString value;
    if (!m_new_data
        && (!m_last_change.IsValid()
            || (m_allowed_age_sec >= 0
                && !wxDateTime::Now().IsEqualUpTo(
                    m_last_change, wxTimeSpan(m_allowed_age_sec))))) {
        value = "-----";
        if (!m_timed_out) {
            needs_redraw = true;
            m_timed_out = true;
        }
    } else {
        m_new_data = false;
        needs_redraw = true;
        m_last_change = wxDateTime::Now();
        const wxJSONValue* val = m_parent_dashboard->GetSKData(m_sk_key);
        if (val) {
            wxJSONValue v = val->Get("value", *val);
            if (m_format_index >= m_format_strings.GetCount()) {
                value = wxString::Format(
                    "E: format", m_format_index, m_format_strings.GetCount());
            } else {
                value = wxString::Format(
                    m_format_strings[m_format_index], Transform(v.AsDouble()));
            }
        } else {
            value = _("Error!");
        }
    }

    if (!needs_redraw) {
        return m_bmp;
    }

    wxString dummy_str(
        "9999"); // dummy string to size the instrument consistently
    wxCoord size_x, size_y;
    wxCoord title_x, title_y;
    wxCoord body_x, body_y;
    wxCoord dummy_x, dummy_y;
    wxMemoryDC dc;

    wxFont tf = m_title_font;
    tf.SetPointSize(m_title_font.GetPointSize() * scale);
    dc.SetFont(tf);
    dc.GetTextExtent(m_title, &title_x, &title_y);
    wxFont bf = m_body_font;
    bf.SetPointSize(m_body_font.GetPointSize() * scale);
    dc.SetFont(bf);
    dc.GetTextExtent(dummy_str, &dummy_x, &dummy_y);
    dc.GetTextExtent(value, &body_x, &body_y);
    size_x = (wxMax(title_x, wxMax(body_x, dummy_x)) + 2 * BORDER_SIZE);
    size_y = (title_y + body_y + 4 * BORDER_SIZE);
    m_bmp = wxBitmap(size_x, size_y);
    dc.SelectObject(m_bmp);
    // Draw stuff
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting("body_background"))));
    dc.DrawRectangle(0, 0, size_x, size_y);
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting("title_background"))));
    dc.DrawRectangle(0, 0, size_x, title_y + 2 * BORDER_SIZE);
    dc.SetFont(tf);
    dc.SetTextForeground(GetDimedColor(GetColorSetting("title_color")));
    dc.DrawText(m_title, (size_x - title_x) / 2, BORDER_SIZE);
    dc.SetFont(bf);
    dc.SetTextForeground(GetDimedColor(GetColorSetting("body_color")));
    dc.DrawText(value, (size_x - body_x) / 2, title_y + 3 * BORDER_SIZE);
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting("border_color")),
        BORDER_LINE_WIDTH, wxPENSTYLE_SOLID));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(BORDER_LINE_WIDTH / 2, BORDER_LINE_WIDTH / 2,
        size_x - BORDER_LINE_WIDTH, size_y - BORDER_LINE_WIDTH);
    // Done drawing
    dc.SelectObject(wxNullBitmap);
    return m_bmp;
}

void SimpleNumberInstrument::ReadConfig(wxJSONValue& config)
{
    Instrument::ReadConfig(config);
#define X(a, b, c, d, e, f, g, h)                                              \
    if (config.HasMember(b)) {                                                 \
        SetSetting(b, config[b].g());                                          \
    }
    DSK_SNI_SETTINGS
#undef X
};

wxJSONValue SimpleNumberInstrument::GenerateJSONConfig()
{
    // Shared parameters from the parent
    wxJSONValue v = Instrument::GenerateJSONConfig();
    // my own parameters
#define X(a, b, c, d, e, f, g, h) v[b] = h(b);
    DSK_SNI_SETTINGS
#undef X
    return v;
};

const double SimpleNumberInstrument::Transform(const double& val)
{
    switch (m_transformation) {
    case transformation::none:
        return val;
    case transformation::rad2deg:
        return rad2deg(val);
    case transformation::ms2kn:
        return 1.943844 * val;
    case transformation::ms2kmh:
        return 3.6 * val;
    case transformation::ms2mph:
        return 2.236936 * val;
    default:
        return val;
    }
};

PLUGIN_END_NAMESPACE