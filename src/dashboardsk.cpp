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

#include "dashboardsk.h"
#include "dashboardsk_pi.h"
#include "wx/jsonwriter.h"
#include <wx/tokenzr.h>

PLUGIN_BEGIN_NAMESPACE

DashboardSK::DashboardSK(const wxString& data_path)
    : m_parent_window(nullptr)
    , m_parent_plugin(nullptr)
    , m_self(wxEmptyString)
    , m_self_ptr(nullptr)
    , m_frozen(false)
    , m_color_scheme(0)
    , m_data_dir(data_path)
{
    for (int i = 0; i < GetCanvasCount(); i++) {
        m_displayed_pages.insert({ i, new Pager(this) });
    }
    m_sk_data["vessels"].AddComment("Root of the vessel tree");
}

void DashboardSK::ProcessData()
{
    for (auto dashboard : m_dashboards) {
        dashboard->ProcessData();
    }
}

int DashboardSK::ToPhys(int x) { return m_parent_plugin->ToPhys(x); }

void DashboardSK::Draw(dskDC* dc, PlugIn_ViewPort* vp, int canvasIndex)
{
    if (m_displayed_pages.find(canvasIndex) == m_displayed_pages.end()) {
        m_displayed_pages[canvasIndex] = new Pager(this);
    }
    m_displayed_pages[canvasIndex]->Draw(dc, vp, canvasIndex);
    Dashboard::ClearOffsets();
    bool drawn = false;
    for (auto dashboard : m_dashboards) {
        if (!m_frozen
            && m_displayed_pages[canvasIndex]->GetCurrentPage()
                == dashboard->GetPageNr()) {
            dashboard->Draw(dc, vp, canvasIndex);
            drawn = true;
        } else {
            dashboard->ProcessData();
        }
    }
    if (!drawn) {
        m_displayed_pages[canvasIndex]->ResetCurrentPage();
    }
}

void DashboardSK::ReadConfig(wxJSONValue& config)
{
    LOG_VERBOSE("DashboardSK_pi: Reading DashboardSK config");
    for (auto db : m_dashboards) {
        delete db;
    }
    m_dashboards.clear();
    if (config["signalk"].HasMember("self")) {
        SetSelf(config["signalk"]["self"].AsString());
    }
    if (!config.HasMember("dashboards")) {
        LOG_VERBOSE("DashboardSK_pi: No dashboards node in JSON");
    }
    if (config["dashboards"].IsArray()) {
        for (int i = 0; i < config["dashboards"].Size(); i++) {
            auto* d = new Dashboard(this);
            d->ReadConfig(config["dashboards"][i]);
            d->SetColorScheme(m_color_scheme);
            m_dashboards.emplace_back(d);
            if (m_displayed_pages.find(d->GetCanvasNr())
                == m_displayed_pages.end()) {
                m_displayed_pages[d->GetCanvasNr()] = new Pager(this);
            }
            m_displayed_pages[d->GetCanvasNr()]->AddPage(d->GetPageNr());
        }
    } else {
        LOG_VERBOSE("DashboardSK_pi: No dashboards array");
    }
    if (config["canvas"].IsArray()) {
        for (int i = 0; i < config["canvas"].Size(); i++) {
            if (m_displayed_pages.find(config["canvas"][i]["id"].AsInt())
                == m_displayed_pages.end()) {
                m_displayed_pages[config["canvas"][i]["id"].AsInt()]
                    = new Pager(this);
            }
            m_displayed_pages[config["canvas"][i]["id"].AsInt()]
                ->SetCurrentPage(config["canvas"][i]["page"].AsInt());
        }
    }
    for (int i = 0; i < GetCanvasCount(); i++) {
        if (m_displayed_pages.find(i) == m_displayed_pages.end()) {
            m_displayed_pages[i] = new Pager(this);
            m_displayed_pages[i]->SetCurrentPage(1);
        }
    }
}

wxJSONValue DashboardSK::GenerateJSONConfig()
{
    wxJSONValue v;
    v["signalk"]["self"] = m_self;
    for (auto dashboard : m_dashboards) {
        v["dashboards"].Append(dashboard->GenerateJSONConfig());
    }
    for (auto page : m_displayed_pages) {
        wxJSONValue vc;
        vc["id"] = page.first;
        vc["page"] = page.second->GetCurrentPage();
        v["canvas"].Append(vc);
    }
    return v;
}

void DashboardSK::SetColorScheme(int cs)
{
    m_color_scheme = cs;
    for (auto dashboard : m_dashboards) {
        dashboard->SetColorScheme(cs);
    }
}

const int DashboardSK::GetColorScheme() { return m_color_scheme; }

const wxJSONValue* DashboardSK::GetSKData(const wxString& path)
{
    wxStringTokenizer tokenizer(path, ".");
    wxJSONValue* ptr = &m_sk_data;
    wxString token;
    while (tokenizer.HasMoreTokens()) {
        token = tokenizer.GetNextToken();
        if (ptr->HasMember(token)) {
            ptr = &(*ptr)[token];
        } else {
            return nullptr; // Not found
        }
    }
    return ptr;
}

void DashboardSK::ProcessComplexValue(wxJSONValue* parent,
    const wxJSONValue& value, const wxDateTime& ts, const wxString& source)
{
    if (value.IsObject()) {
        wxArrayString vals = value.GetMemberNames();
        for (wxString val : vals) {
            (*parent)[val] = wxJSONValue();
            ProcessComplexValue(
                &(*parent)[val], value.Get(val, wxJSONValue()), ts, source);
        }
    } else {
        (*parent)["value"] = value;
        (*parent)["timestamp"] = ts.FormatISOCombined();
        (*parent)["source"] = source;
    }
}

void DashboardSK::SendSKDelta(wxJSONValue& message)
{
    LOG_RECEIVE("Received SK message: " + message.AsString());
    if (m_self.IsEmpty() && message.HasMember("self")) {
        // If we still don't have Self ID set, we accept it from the core
        // TODO: We perhaps might allow this until the user ever modifies it
        // manually in the prefs
        LOG_RECEIVE_DEBUG(
            "Message contains self indentifier " + message["self"].AsString());
        SetSelf(message["self"].AsString());
    }
    wxString fullKey;
    if (!message.HasMember("context")) {
        LOG_RECEIVE(
            "Message does not contain context " + message.GetMemberNames()[0]);
        if (!message.HasMember("updates") || !message["updates"].IsArray()) {
            LOG_RECEIVE("Message does not look OK");
            return; // Invalid SK delta
        }
        fullKey = "vessels.self";
    } else {
        fullKey = message["context"].AsString();
    }
    LOG_RECEIVE_DEBUG("Message seems OK");

    int skip_tokens = 0;
    wxJSONValue* ptr;
    if (fullKey.StartsWith("vessels.self")) {
        // "vessels.self" translated to fully qualified identified ID and we can
        // skip the first two levels directly
        ptr = m_self_ptr;
        fullKey.Replace("vessels.self", "vessels." + Self());
        skip_tokens = 2;
    } else {
        ptr = &m_sk_data;
        fullKey = wxEmptyString;
    }
    LOG_RECEIVE_DEBUG("Full key before parsing: " + fullKey);
    wxStringTokenizer ctx_tokenizer(message["context"].AsString(), ".");
    int token_nr = 0;
    wxString token;
    while (ctx_tokenizer.HasMoreTokens()) {
        ++token_nr;
        token = ctx_tokenizer.GetNextToken();
        if (token_nr > skip_tokens) {
            if (fullKey == wxEmptyString) {
                fullKey = token;
            } else {
                if (token_nr == 2) {
                    fullKey.Append(".").Append(NormalizeID(token));
                } else {
                    fullKey.Append(".").Append(token);
                }
            }

            if (!ptr->HasMember(token)) {
                LOG_RECEIVE_DEBUG(ptr->GetInfo() + " does NOT have member "
                    + token + ", adding it");
                (*ptr)[token] = wxJSONValue();
                ptr = &(*ptr)[token];
            } else {
                LOG_RECEIVE_DEBUG(ptr->GetInfo() + " does have member " + token
                    + ", reusing it");
                ptr = &(*ptr)[token];
            }
        } else {
            skip_tokens--;
        }
    }
    LOG_RECEIVE_DEBUG("Full key after parsing: " + fullKey);
    wxDateTime ts;
    for (int i = 0; i < message["updates"].Size(); i++) {
        LOG_RECEIVE_DEBUG("processing update #%i", i);
        if (message["updates"][i].HasMember("timestamp")) {
            if (!ts.ParseISOCombined(
                    message["updates"][i]["timestamp"].AsString())) {
                ts = wxDateTime::Now();
            }
        } else {
            ts = wxDateTime::Now();
        }
        // TODO: Some deltas may contain timestamp also as a value (ex.
        // position.timestamp), we could sometimes use or maybe even prefer them
        wxString fullKeyWithPath;
        wxString source = wxEmptyString;
        if (message["updates"][i].HasMember("$source")) {
            source = message["updates"][i]["$source"].AsString();
        } else if (message["updates"][i].HasMember("source")) {
            if (message["updates"][i]["source"]["type"].AsString()
                == "NMEA0183") {
                source
                    .Append(message["updates"][i]["source"]["label"].AsString())
                    .Append("-")
                    .Append(
                        message["updates"][i]["source"]["talker"].AsString())
                    .Append("-")
                    .Append(
                        message["updates"][i]["source"]["sentence"].AsString());
            } else if (message["updates"][i]["source"]["type"].AsString()
                == "NMEA2000") {
                source
                    .Append(message["updates"][i]["source"]["label"].AsString())
                    .Append("-")
                    .Append(message["updates"][i]["source"]["pgn"].AsString());
            } else {
                source.Append(
                    message["updates"][i]["source"]["label"].AsString());
            }
        }
        wxString utoken;
        if (message["updates"][i].HasMember("values")) {
            for (int j = 0; j < message["updates"][i]["values"].Size(); j++) {
                wxJSONValue* val_ptr = ptr;
                fullKeyWithPath = fullKey;
                LOG_RECEIVE_DEBUG("processing value #%i (%s) under %s", j,
                    message["updates"][i]["values"][j]["path"].AsCString(),
                    fullKeyWithPath.c_str());
                wxStringTokenizer path_tokenizer(
                    message["updates"][i]["values"][j]["path"].AsString(), ".");
                while (path_tokenizer.HasMoreTokens()) {
                    utoken = path_tokenizer.GetNextToken();
                    fullKeyWithPath.Append(".").Append(utoken);
                    if (!val_ptr->HasMember(utoken)
                        && path_tokenizer.HasMoreTokens()) {
                        // If we are not done parsing the path yet, we add
                        // another branch if needed
                        LOG_RECEIVE_DEBUG(fullKeyWithPath
                            + " not yet in the tree, adding " + utoken);
                        (*val_ptr)[utoken] = wxJSONValue();
                        val_ptr = &(*val_ptr)[utoken];
                    } else {
                        LOG_RECEIVE_DEBUG(
                            fullKeyWithPath + " already exists in the tree");
                        val_ptr = &(*val_ptr)[utoken];
                    }
                }
                if (!message["updates"][i]["values"][j]["value"].IsNull()) {
                    // We ignore NULL values received from SignalK
                    // TODO: Are some NULLs in SignalK data actually good for
                    // something? (If they are, we want to ignore them later
                    // selectively when the instrument processes it's data)
                    if (!source.IsEmpty()) {
                        wxString src_key = SRC_MAGIC_STRING + source;
                        src_key.Replace(".", "-", true);
                        (*val_ptr)[src_key] = wxJSONValue();
                        val_ptr = &(*val_ptr)[src_key];
                    }
                    ProcessComplexValue(val_ptr,
                        message["updates"][i]["values"][j]["value"], ts,
                        source);

                    LOG_RECEIVE_DEBUG(
                        "Notifying update to path " + fullKeyWithPath);
                    for (auto instr :
                        m_path_subscriptions[UNORDERED_KEY(fullKeyWithPath)]) {
                        instr->NotifyNewData(fullKeyWithPath);
                    }
                }
            }
        } else if (message["updates"][i].HasMember("meta")) {
            for (int j = 0; j < message["updates"][i]["meta"].Size(); j++) {
                wxJSONValue* val_ptr = ptr;
                fullKeyWithPath = fullKey;
                LOG_RECEIVE_DEBUG("processing value #%i (%s) under %s", j,
                    message["updates"][i]["meta"][j]["path"].AsCString(),
                    fullKeyWithPath.c_str());
                wxStringTokenizer path_tokenizer(
                    message["updates"][i]["meta"][j]["path"].AsString(), ".");
                while (path_tokenizer.HasMoreTokens()) {
                    utoken = path_tokenizer.GetNextToken();
                    fullKeyWithPath.Append(".").Append(utoken);
                    if (!val_ptr->HasMember(utoken)
                        && path_tokenizer.HasMoreTokens()) {
                        // If we are not done parsing the path yet, we add
                        // another branch if needed
                        LOG_RECEIVE_DEBUG(fullKeyWithPath
                            + " not yet in the tree, adding " + utoken);
                        (*val_ptr)[utoken] = wxJSONValue();
                        val_ptr = &(*val_ptr)[utoken];
                    } else {
                        LOG_RECEIVE_DEBUG(
                            fullKeyWithPath + " already exists in the tree");
                        val_ptr = &(*val_ptr)[utoken];
                    }
                }
                if (!message["updates"][i]["meta"][j].IsNull()) {
                    (*val_ptr)["meta"]
                        = message["updates"][i]["meta"][j]["value"];
                }
            }
        }
    }
}

wxString DashboardSK::GetSignalKTreeText()
{
    wxJSONWriter w;
    wxString s;
    w.Write(m_sk_data, s);
    return s;
}

wxJSONValue* DashboardSK::GetSignalKTree() { return &m_sk_data; }

const wxString DashboardSK::SelfTranslate(const wxString& path)
{
    if (Self().IsEmpty()) {
        return path;
    }
    wxString new_path = path;
    new_path.Replace("." + Self(), ".self");
    return new_path;
}

const wxString DashboardSK::SelfPopulate(const wxString& path)
{
    if (Self().IsEmpty()) {
        return path;
    }
    wxString new_path = path;
    new_path.Replace(".self", "." + Self());
    return new_path;
}

void DashboardSK::AddPageToCanvas(const int& canvas, const size_t& page)
{
    if (canvas < GetCanvasCount()) {
        m_displayed_pages[canvas]->AddPage(page);
    }
}

bool DashboardSK::ProcessMouseEvent(wxMouseEvent& event)
{
    // Pagers
    for (auto page : m_displayed_pages) {
        if (GetCanvasIndexUnderMouse() == page.first
            && page.second->ProcessMouseEvent(event)) {
            return true;
        }
    }
    // TODO: Pass to the dashboards for interactive instruments
    return false;
}

wxBitmap DashboardSK::ApplyBitmapBrightness(wxBitmap& bitmap)
{
    double dimLevel;
    switch (m_color_scheme) {
    case PI_GLOBAL_COLOR_SCHEME_DUSK: {
        dimLevel = 0.8;
        break;
    }
    case PI_GLOBAL_COLOR_SCHEME_NIGHT: {
        dimLevel = 0.5;
        break;
    }
    default: {
        return bitmap;
    }
    }

    return SetBitmapBrightnessAbs(bitmap, dimLevel);
}

wxBitmap DashboardSK::SetBitmapBrightnessAbs(wxBitmap& bitmap, double level)
{
    wxImage image = bitmap.ConvertToImage();

    int gimg_width = image.GetWidth();
    int gimg_height = image.GetHeight();

    for (int iy = 0; iy < gimg_height; iy++) {
        for (int ix = 0; ix < gimg_width; ix++) {
            if (!image.IsTransparent(ix, iy, 30)) {
                wxImage::RGBValue rgb(image.GetRed(ix, iy),
                    image.GetGreen(ix, iy), image.GetBlue(ix, iy));
                wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
                hsv.value = hsv.value * level;
                wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
                image.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
            }
        }
    }
    return wxBitmap(image);
}

void DashboardSK::ResetPagers()
{
    for (auto& page : m_displayed_pages) {
        page.second->Reset();
        for (auto& dashboard : m_dashboards) {
            if (page.first == dashboard->GetCanvasNr()) {
                page.second->AddPage(dashboard->GetPageNr());
            }
        }
    }
}

void DashboardSK::SetParentPlugin(dashboardsk_pi* parent)
{
    m_parent_plugin = parent;
}

void DashboardSK::ShowPreferencesDialog()
{
    m_parent_plugin->ShowPreferencesDialog(m_parent_window);
}

void DashboardSK::ToggleVisibility()
{
    m_parent_plugin->OnToolbarToolCallback(0);
}

bool DashboardSK::IsVisible() { return m_parent_plugin->IsVisible(); }

double DashboardSK::GetContentScaleFactor() const
{
    return m_parent_plugin->GetContentScaleFactor();
}

PLUGIN_END_NAMESPACE
