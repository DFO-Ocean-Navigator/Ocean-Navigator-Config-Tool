#ifndef DIALOGPREFERENCES_H
#define DIALOGPREFERENCES_H

#include "defines.h"

#include "preferences.h"

#include <QDialog>
#include <QSettings>

/***********************************************************************************/
// Forward Declarations
namespace Ui {
class DialogPreferences;
}

/***********************************************************************************/
class DialogPreferences : public QDialog {
	Q_OBJECT

public:
	explicit DialogPreferences(QWidget* parent = nullptr);
	~DialogPreferences();

	void SetPreferences(Preferences& settings);
	auto GetPreferences() const noexcept -> Preferences NODISCARD;

private slots:
	void on_pushButtonBrowseInstallDir_clicked();

private:
	Ui::DialogPreferences* m_ui{nullptr};


};

#endif // DIALOGPREFERENCES_H
