#include <wx/wx.h>
#include <wx/intl.h>
#include "Database.h"
#include "MainFrame.h"
#include <memory>

class MyApp : public wxApp {
public:
    bool OnInit() override {
        // Настройка локализации
        wxLocale* locale = new wxLocale();
        locale->Init(wxLANGUAGE_RUSSIAN, wxLOCALE_LOAD_DEFAULT);

        // Добавляем каталог с файлами перевода (если они есть)
        // locale->AddCatalog("db_manager");

        try {
            auto db = std::make_shared<Database>("mydb.sqlite");
            MainFrame* frame = new MainFrame(db);
            frame->Show(true);
        }
        catch (const DbException& e) {
            wxMessageBox(e.what(), _("Ошибка БД"), wxOK | wxICON_ERROR);
            return false;
        }
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);