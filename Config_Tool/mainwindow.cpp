#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

/***********************************************************************************/
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_ui(new Ui::MainWindow) {
	m_ui->setupUi(this);

	setWindowTitle(tr("Ocean Navigator Dataset Config Editor"));

	// Set dark stylesheet
	QFile f(":qdarkstyle/style.qss");
	if (!f.exists()) {
		qDebug() << "Unable to set stylesheet, file not found.";
	}
	else {
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		qApp->setStyleSheet(ts.readAll());
	}

	statusBar()->showMessage(tr("Open a config file from File -> Open, or use CTRL + O."));

	m_ui->labelList->setText(tr("Double-click on a dataset to edit it's properties, or click on the Add Dataset button."));
	m_ui->buttonAddDataset->setText(tr("Add Dataset"));
}

/***********************************************************************************/
MainWindow::~MainWindow() {
	delete m_ui;
}

/***********************************************************************************/
void MainWindow::on_actionAbout_Qt_triggered() {
	QMessageBox::aboutQt(this);
}

/***********************************************************************************/
void MainWindow::on_actionOpen_triggered() {
	QFileDialog dialog(this);
	dialog.setWindowTitle(tr("Open Dataset Config File"));
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter(tr("Config Files (*.json)"));
	dialog.setViewMode(QFileDialog::Detail);
	dialog.setDirectory("/home/nabil/");

	// Open file dialog
	if (dialog.exec()) {
		m_configFileName = dialog.selectedFiles().at(0); // Get selected file.
	}

	// If a file was actually selected (ignore cancel or close)
	if (!m_configFileName.isEmpty()) {
		// Open file
		QFile f(m_configFileName);
		f.open(QFile::ReadOnly | QFile::Text);
		const QString contents = f.readAll();
		f.close();

		// Parse json
		// https://stackoverflow.com/questions/15893040/how-to-create-read-write-json-files-in-qt5
		const auto jsonDocument = QJsonDocument::fromJson(contents.toUtf8());
		m_documentRootObject = jsonDocument.object(); // Get copy of root object

		statusBar()->showMessage("");
	}
}

/***********************************************************************************/
void MainWindow::on_actionClose_triggered() {

	if (m_isUnsavedData) {
		// show confirm quit dialog
	}

	close();
}


/***********************************************************************************/
void MainWindow::on_actionSave_triggered() {

	if (!m_configFileName.isEmpty()) {
		const QJsonDocument doc(m_documentRootObject);

		QSaveFile f("/home/nabil/test.txt");
		f.open(QFile::WriteOnly | QFile::Text | QFile::Truncate); // Overwrite original file.
		f.write(doc.toJson());
		f.commit();

		statusBar()->showMessage(tr("Config file saved!"));
	}
}

/***********************************************************************************/
void MainWindow::on_buttonAddDataset_clicked() {
	m_isUnsavedData = true;
}

/***********************************************************************************/
void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem* item) {
	qDebug() << item->text();
}
