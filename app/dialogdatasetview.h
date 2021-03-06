#ifndef DATASETVIEW_H
#define DATASETVIEW_H

#include "constants.h"
#include "datadownloaddesc.h"

#include <QDialog>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QString>

#include <utility>

/***********************************************************************************/
// Forward Declarations
namespace Ui {
class DatasetView;
}
class HTMLHighlighter;

/***********************************************************************************/
/// Dialog window to show dataset attributes (THREDDS URL, variables, etc).
class DialogDatasetView : public QDialog {
  Q_OBJECT

public:
  explicit DialogDatasetView(QWidget *parent = nullptr);
  ~DialogDatasetView() override;

  /// Parse data from loaded config file
  void SetData(const QString &datasetKey, const QJsonObject &object);
  /// Queries the remove server for dataset attributes.
  void SetData(const QJsonObject &datasetObj, QNetworkAccessManager &nam);
  /// Fill the UI elements from attributes in DataDownloadDesc.
  void SetData(const DataDownloadDesc &data);

  /// Serializes values from the UI into a JSON object.
  /** Returns the dataset key, and JSON object. */
  NODISCARD std::pair<QString, QJsonObject> GetData() const;

  /// Constructs a DataDownloadDesc from values in UI fields.
  NODISCARD DataDownloadDesc GetDownloadData() const;

private slots:
  void on_pushButtonAddVariable_clicked();
  void on_pushButtonDeleteVariable_clicked();
  void on_lineEditKey_editingFinished();
  void on_lineEditName_editingFinished();
  void on_lineEditURL_editingFinished();

  void keyPressEvent(QKeyEvent *e) override;
  void on_pushButtonMagicScan_clicked();

private:
  /// Adds a new variable row with empty values. Returns new row index.
  int addEmptyVariable();
  ///
  void checkInputEmpty(const QString &inputLabel, const QString &inputText);
  ///
  void setReadOnlyUI();

  Ui::DatasetView *m_ui{nullptr};
  HTMLHighlighter *m_highlighter{nullptr};

  QHash<QString, QString> m_variableMap;
};

#endif // DATASETVIEW_H
