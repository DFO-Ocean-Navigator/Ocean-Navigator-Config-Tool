#include "dialogpreferences.h"
#include "ui_dialogpreferences.h"

#include "ioutils.h"

#include <QFileDialog>
#include <QMessageBox>

/***********************************************************************************/
DialogPreferences::DialogPreferences(QWidget* parent) : QDialog{parent},
														m_ui{new Ui::DialogPreferences} {
	m_ui->setupUi(this);

	const QStringList downloadFormats {
		"NETCDF4", "NETCDF4_CLASSIC", "NETCDF3_64BIT",
		"NETCDF3_CLASSIC", "NETCDF3_NC"
	};

	m_ui->comboBoxDownloadFormat->addItems(downloadFormats);
}

/***********************************************************************************/
DialogPreferences::~DialogPreferences() {
	delete m_ui;
}

/***********************************************************************************/
void DialogPreferences::SetPreferences(Preferences& prefs) {
	m_ui->lineEditInstallDir->setText(prefs.ONInstallDir);
	m_ui->lineEditRemoteURL->setText(prefs.RemoteURL);
	m_ui->lineEditTHREDDSDataLocation->setText(prefs.THREDDSCatalogLocation);
	m_ui->comboBoxDownloadFormat->setCurrentText(prefs.DataDownloadFormat);
	m_ui->switchUpdateDoryDatasetsOnStart->setChecked(prefs.UpdateRemoteListOnStart);
	m_ui->switchAutoStartServers->setChecked(prefs.AutoStartServers);
	m_ui->switchOnlineOffline->setChecked(prefs.IsNetworkOnline);
	m_ui->switchWidgetCheckForUpdates->setChecked(prefs.CheckForUpdatesOnStart);
}

/***********************************************************************************/
auto DialogPreferences::GetPreferences() const -> Preferences {
	return {
		m_ui->lineEditInstallDir->text(),
		m_ui->lineEditRemoteURL->text(),
		m_ui->lineEditTHREDDSDataLocation->text(),
		m_ui->comboBoxDownloadFormat->currentText(),
		m_ui->switchUpdateDoryDatasetsOnStart->isChecked(),
		m_ui->switchAutoStartServers->isChecked(),
		m_ui->switchOnlineOffline->isChecked(),
		m_ui->switchWidgetCheckForUpdates->isChecked()
	};
}

/***********************************************************************************/
void DialogPreferences::on_pushButtonBrowseInstallDir_clicked() {
#ifdef QT_DEBUG
	const constexpr auto path = "/home/nabil/";
#else
	const constexpr auto path = "/opt/tools/";
#endif
	const auto& dir{ QFileDialog::getExistingDirectory(this,
													   tr("Open Ocean Navigator Installation Folder..."),
													   path,
													   QFileDialog::ShowDirsOnly)
				   };

	if (!dir.isEmpty()) {
		m_ui->lineEditInstallDir->setText(dir);
	}
}

/***********************************************************************************/
void DialogPreferences::on_pushButtonBroweseTHREDDS_clicked() {
	const auto& dir{ QFileDialog::getExistingDirectory(this, tr("Open THREDDS catalog.xml folder...")) };

	if (!dir.isEmpty()) {
		if (IO::FileExists(dir+"/catalog.xml")) {
			m_ui->lineEditTHREDDSDataLocation->setText(dir);
			return;
		}

		QMessageBox::critical(this,
							  tr("Error..."),
							  tr("catalog.xml not found in this directory: ") +
							  dir
							  );
	}
}
