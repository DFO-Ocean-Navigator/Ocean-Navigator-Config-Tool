#include "widgetthreddsconfig.h"
#include "ui_widgetthreddsconfig.h"

#include "preferences.h"
#include "constants.h"
#include "ioutils.h"

#include <QFile>
#include <QPushButton>
#include <QMessageBox>
#include <QDir>
#include <QInputDialog>
#include <QRegularExpression>

#include <pugixml/pugixml.hpp>

#include <cstring>
#include <optional>

#ifdef QT_DEBUG
	#include <QDebug>
#endif

/***********************************************************************************/
NODISCARD auto readXML(const QString& path) {
	pugi::xml_document doc;

	const auto& result{ doc.load_file(path.toStdString().c_str()) };

	return result ? std::make_optional(std::move(doc)) : std::nullopt;
}

/***********************************************************************************/
// Creates a new catalog document with required headers/attributes
NODISCARD auto createNewCatalogFile() {
	pugi::xml_document doc;

	// Header declaration
	auto header{ doc.prepend_child(pugi::node_declaration) };
	header.append_attribute("version") = 1.0;
	header.append_attribute("encoding") = "UTF-8";

	// Top-level <catalog></catalog> tags
	auto catalog{ doc.append_child() };
	catalog.set_name("catalog");
	catalog.append_attribute("xmlns") = "http://www.unidata.ucar.edu/namespaces/thredds/InvCatalog/v1.0";
	catalog.append_attribute("xmlns:xlink") = "http://www.w3.org/1999/xlink";
	catalog.append_attribute("name") = "Unidata THREDDS-IDD NetCDF-OpenDAP Server";
	catalog.append_attribute("version") = "1.0.1";

	{
		// Service tags
		auto services{ catalog.append_child() };
		services.set_name("service");
		services.append_attribute("name") = "all";
		services.append_attribute("base") = "";
		services.append_attribute("serviceType") = "compound";


		auto odapService{ services.append_child( ) };
		odapService.set_name("service");
		odapService.append_attribute("name") = "odap";
		odapService.append_attribute("serviceType") = "OpenDAP";
		odapService.append_attribute("base") = "/thredds/dodsC/";

		auto dap4Service{ services.append_child( ) };
		dap4Service.set_name("service");
		dap4Service.append_attribute("name") = "dap4";
		dap4Service.append_attribute("serviceType") = "DAP4";
		dap4Service.append_attribute("base") = "/thredds/dap4/";

		auto httpService{ services.append_child() };
		httpService.set_name("service");
		httpService.append_attribute("name") = "http";
		httpService.append_attribute("serviceType") = "HTTPServer";
		httpService.append_attribute("base") = "/thredds/fileServer/";

		auto ncssService{ services.append_child() };
		ncssService.set_name("service");
		ncssService.append_attribute("name") = "ncss";
		ncssService.append_attribute("serviceType") = "NetcdfSubset";
		ncssService.append_attribute("base") = "/thredds/ncss/";
	}

	{
		auto service{ catalog.append_child() };
		service.set_name("service");
		service.append_attribute("name") = "dap";
		service.append_attribute("serviceType") = "compound";
		service.append_attribute("base") = "";

		auto odapService{ service.append_child() };
		odapService.set_name("service");
		odapService.append_attribute("name") = "odap";
		odapService.append_attribute("serviceType") = "OpenDAP";
		odapService.append_attribute("base") = "/thredds/dodsC/";

		auto dap4Service{ service.append_child( ) };
		dap4Service.set_name("service");
		dap4Service.append_attribute("name") = "dap4";
		dap4Service.append_attribute("serviceType") = "DAP4";
		dap4Service.append_attribute("base") = "/thredds/dap4/";
	}

	return doc;
}

/***********************************************************************************/
// Creates a new empty aggregate file with required tags.
NODISCARD auto createNewAggregateFile() {
	pugi::xml_document doc;

	auto netCDFRoot{ doc.append_child() };
	netCDFRoot.set_name("netcdf");
	netCDFRoot.append_attribute("xmlns") = "http://www.unidata.ucar.edu/namespaces/netcdf/ncml-2.2";

	return doc;
}

/***********************************************************************************/
WidgetThreddsConfig::WidgetThreddsConfig(QWidget* parent, const Preferences* prefs) :	QWidget{parent},
																						m_ui{new Ui::WidgetThreddsConfig},
																						m_prefs{prefs} {
	m_ui->setupUi(this);

	m_ui->tableWidget->horizontalHeader()->setStretchLastSection(true); // Resize columns to widget width
	m_ui->tableWidget->setHorizontalHeaderLabels({
													tr("Dataset Name"),
													tr("Catalog Path"),
													tr("Dataset Files Location")
												 });

	m_ui->labelTHREDDSPath->setText(tr("THREDDS Location: ") + m_prefs->THREDDSCatalogLocation);
	m_ui->labelTHREDDSPath->setStyleSheet(QSS_COLOR_GREEN);

	checkCatalogsPath();

	buildTable();
}

/***********************************************************************************/
WidgetThreddsConfig::~WidgetThreddsConfig() {
	delete m_ui;
}

/***********************************************************************************/
void WidgetThreddsConfig::on_tableWidget_cellChanged(int row, int column) {
	switch (column) {
	case 0:
		if (validateDatasetName(m_ui->tableWidget->item(row, 0)->text())) {
			m_ui->tableWidget->item(row, 0)->setTextColor(QColor(Qt::white));
		}
		else {
			m_ui->tableWidget->item(row, 0)->setTextColor(QColor(COLOR_RED));
		}
		break;
	default:
		break;
	}
}

/***********************************************************************************/
void WidgetThreddsConfig::on_pushButtonAddDataset_clicked() {

	bool ok_dsname;
	const auto& datasetName{ QInputDialog::getText(	this,
													tr("Add THREDDS Dataset..."),
													tr("Enter a dataset name.\nNo whitespace, /, \\, +, *, ., characters allowed."),
													QLineEdit::Normal,
													"my_dataset_" + QString::number(qrand()),
													&ok_dsname)
						   };

	bool ok_dspath;
	const auto& datasetPath{ QInputDialog::getText( this,
												   tr("Dataset Path..."),
												   tr("Enter the path for the folder containing the dataset."),
												   QLineEdit::Normal,
												   "",
												   &ok_dspath)
						  };

	if (ok_dsname && validateDatasetName(datasetName) && ok_dspath) {
		createRow(datasetName, datasetPath);
		addDataset(datasetName, datasetPath);

		QMessageBox::information(this,
								 tr("Success..."),
								 tr("Your dataset has been added to THREDDS! Go to the Dashboard to Stop and Start the Apache server to apply your changes."));
	}
}

/***********************************************************************************/
void WidgetThreddsConfig::on_pushButtonRemoveDataset_clicked() {
	const auto currentRow{ m_ui->tableWidget->currentRow() };

	if (currentRow != -1) {

		const auto reply{ QMessageBox::warning(this,
											   tr("Confirm Action"),
											   tr("Delete selected dataset? Warning: this will remove the netCDF files too!"),
											   QMessageBox::Yes | QMessageBox::No)
						};

		if (reply == QMessageBox::Yes) {
			removeDataset(m_ui->tableWidget->item(currentRow, 0)->text(),
						  m_ui->tableWidget->item(currentRow, 2)->text()
						  );

			m_ui->tableWidget->removeRow(m_ui->tableWidget->currentRow());
		}
	}
}

/***********************************************************************************/
void WidgetThreddsConfig::buildTable() {

	const auto& catalogFile{ m_prefs->THREDDSCatalogLocation + QString("/catalog.xml") };

	const auto doc{ readXML(catalogFile) };

	if (!doc.has_value()) {
		QMessageBox box{this};
		box.setWindowTitle(tr("THREDDS catalog error..."));
		box.setText(tr("Failed to load THREDDS catalog file: ") + catalogFile);
		box.setIcon(QMessageBox::Critical);

		box.exec();

		return;
	}

	const auto& rootNode{ doc->child("catalog") };

	if (!rootNode.select_node("catalogRef")) {
		// Check if the primary catalog doesn't have any dataset references
		return;
	}

	for (const auto& node : rootNode.children()) {
		// Resort to manual comparison since pugi is only listing the first catalog
		// when using node.next_sibling() *shrug*
		if (std::strcmp(node.name(), "catalogRef") == 0) {

			m_ui->tableWidget->insertRow(m_ui->tableWidget->rowCount());
			const auto rowIdx{ m_ui->tableWidget->rowCount() - 1 };

			auto* const nameItem{ new QTableWidgetItem(node.attribute("xlink:title").as_string()) };

			const auto& catalogPath{ node.attribute("xlink:href").as_string() };
			auto* const pathItem{ new QTableWidgetItem(catalogPath) };
			pathItem->setFlags(pathItem->flags() ^ Qt::ItemIsEditable); // Make read-only

			m_ui->tableWidget->setItem(rowIdx, 0, nameItem);
			m_ui->tableWidget->setItem(rowIdx, 1, pathItem);


			const auto& datasetCatalog{ readXML(m_prefs->THREDDSCatalogLocation + "/" + catalogPath) };
			if (!datasetCatalog.has_value()) {
				continue;
			}

			const auto& datasetScanNode{ datasetCatalog->child("catalog").child("datasetScan") };
			if (!datasetScanNode) {
				continue;
			}

			auto* const locationItem{ new QTableWidgetItem(datasetScanNode.attribute("location").as_string()) };
			m_ui->tableWidget->setItem(rowIdx, 2, locationItem);
		}
	}
}

/***********************************************************************************/
void WidgetThreddsConfig::checkCatalogsPath() {
	const QDir dir{m_prefs->THREDDSCatalogLocation+"/catalogs/"};
	if (!dir.exists()) {
#ifdef QT_DEBUG
	qDebug() << "Creating catalogs folder at: " << m_prefs->THREDDSCatalogLocation << "/catalogs/";
#endif
		if (!dir.mkpath(".")) {
			QMessageBox::critical(this,
								  tr("Permissions error..."),
								  tr("Failed to create the catalogs folder for THREDDS. This is probably a permissions issue. Ensure you have write permissions to: ")+
								  m_prefs->THREDDSCatalogLocation);
		}
	}
}

/***********************************************************************************/
bool WidgetThreddsConfig::validateDatasetName(const QString& datasetName) {
	// https://regex101.com/r/2MfejI/3
	const QRegularExpression re{"[\\.,;'`+*\\s/]+", QRegularExpression::CaseInsensitiveOption};

	const auto& matchResult{ re.match(datasetName) };

	return !matchResult.hasMatch();
}

/***********************************************************************************/
void WidgetThreddsConfig::createRow(const QString& datasetName, const QString& dataPath) {

	m_ui->tableWidget->insertRow(m_ui->tableWidget->rowCount());
	const auto rowIdx{ m_ui->tableWidget->rowCount() - 1 };

	auto* const nameItem{ new QTableWidgetItem(datasetName) };

	const auto& catalogPath{ "catalogs/"+datasetName+".xml" };
	auto* const pathItem{ new QTableWidgetItem(catalogPath) };

	auto* const dataPathItem{ new QTableWidgetItem(dataPath) };

	m_ui->tableWidget->setItem(rowIdx, 0, nameItem);
	m_ui->tableWidget->setItem(rowIdx, 1, pathItem);
	m_ui->tableWidget->setItem(rowIdx, 2, dataPathItem);
}

/***********************************************************************************/
void WidgetThreddsConfig::addDataset(const QString& datasetName, const QString& dataPath) {

	const auto& path{ QString("catalogs/") + datasetName };

	// Modify catalog.xml
	{
		const auto& catalogPath{ m_prefs->THREDDSCatalogLocation + QString("/catalog.xml") };
		const auto doc{ readXML(catalogPath) };
		auto child{ doc->child("catalog").append_child("catalogRef") };
		child.append_attribute("xlink:title") = datasetName.toStdString().c_str();
		child.append_attribute("xlink:href") = path.toStdString().c_str();

		doc->save_file(catalogPath.toStdString().c_str());
	}

	const auto& fileName{ "/home/nabil/" + path + ".xml"};
	if (!IO::FileExists(fileName)) {
		IO::CreateDir(fileName);
	}

	// Create dataset catalog file (giops_day.xml for example)
	auto catalog{ readXML(path) };
	if (!catalog.has_value()) {
		catalog = createNewCatalogFile();

		auto datasetScan{ catalog->child("catalog").append_child() };
		datasetScan.set_name("datasetScan");
		datasetScan.append_attribute("name") = datasetName.toStdString().c_str();
		datasetScan.append_attribute("ID") = datasetName.toLower().toStdString().c_str();
		datasetScan.append_attribute("path") = datasetName.toLower().toStdString().c_str();
		datasetScan.append_attribute("location") = dataPath.toStdString().c_str();

		auto serviceName{ datasetScan.append_child() };
		serviceName.set_name("serviceName");
		serviceName.text().set("all");

		catalog->save_file(fileName.toStdString().c_str());
	}

	// Create dataset aggregate file
	const auto aggregatePath{ dataPath + "/aggregated.ncml "};
	auto aggregate{ readXML(aggregatePath) };
	if (!aggregate.has_value()) {
		aggregate = createNewAggregateFile();

		auto aggregation{ aggregate->child("netcdf").append_child() };
		aggregation.set_name("aggregation");
		aggregation.append_attribute("type") = "joinExisting";
		aggregation.append_attribute("recheckEvery") = "1 hour";

		// Find netCDF files in target directory
		QDir dir{dataPath};
		dir.setNameFilters({"*.nc"});

		// Find the name of time dimension of first file.
		// I assume the folder contains netCDF files for the same dataset.
		aggregation.append_attribute("dimName") = IO::FindTimeDimension(dir.entryInfoList()[0].absoluteFilePath()).toStdString().c_str();

		auto scan{ aggregation.append_child() };
		scan.set_name("scan");
		scan.append_attribute("location") = ".";
		scan.append_attribute("regExp") = "^.*\\.nc$";
		scan.append_attribute("recheckEvery") = "1 hour";

		aggregate->save_file(aggregatePath.toStdString().c_str());
	}
}

/***********************************************************************************/
void WidgetThreddsConfig::removeDataset(const QString& datasetName, const QString& dataPath) {

	// Modify catalog.xml
	{
		const auto& catalogPath{ m_prefs->THREDDSCatalogLocation + QString("/catalog.xml") };
		auto doc{ readXML(catalogPath) };
		const auto nodeToRemove{ doc->find_child_by_attribute("catalogRef", "xlink:title", datasetName.toStdString().c_str()) };
		doc->remove_child(nodeToRemove);

		doc->save_file(catalogPath.toStdString().c_str());
	}

	// Remove dataset catalog
	QFile::remove("catalogs/" + datasetName + ".xml");

	// Remove datasets + aggregated file
	IO::RemoveDir(dataPath);
}