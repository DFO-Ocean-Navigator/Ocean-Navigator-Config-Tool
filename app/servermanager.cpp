#include "servermanager.h"

#include "ioutils.h"

#include <QTimer>

/***********************************************************************************/
ServerManager::ServerManager(QObject* parent) : QObject{parent} {
	startServers();
}

/***********************************************************************************/
ServerManager::~ServerManager() {
	stopServers();
}

/***********************************************************************************/
void ServerManager::refreshServers() {
	stopServers();
	QTimer::singleShot(5000, this, &ServerManager::startServers); // Start servers after 5 secs
}

/***********************************************************************************/
void ServerManager::startServers() {
	startWebServer();
	startTHREDDS();
}

/***********************************************************************************/
void ServerManager::startWebServer() {
	m_gunicornProcess.setWorkingDirectory(IO::NAVIGATOR_FRONTEND_DIR);
	m_gunicornProcess.setProgram(QStringLiteral("/bin/sh"));
	m_gunicornProcess.setArguments({QStringLiteral("runserver.sh"), QStringLiteral("datasetconfigOFFLINE.json")});

	m_isGunicornRunning = m_gunicornProcess.startDetached(&m_gunicornPID);

	if (!m_isGunicornRunning) {

	}
}

/***********************************************************************************/
void ServerManager::startTHREDDS() {
	m_apacheProcess.setWorkingDirectory(IO::TOMCAT_BIN_DIR);
	m_apacheProcess.setProgram(QStringLiteral("/bin/sh"));
	m_apacheProcess.setArguments({QStringLiteral("startup.sh")});

	m_isApacheRunning = m_apacheProcess.startDetached();

	if (!m_isApacheRunning) {

	}
}

/***********************************************************************************/
void ServerManager::stopServers() {
	stopWebServer();
	stopTHREDDS();
}

/***********************************************************************************/
void ServerManager::stopWebServer() {
	if (m_isGunicornRunning) {
#ifdef __linux__
		m_isGunicornRunning = QProcess::startDetached(QStringLiteral("pkill"), {QStringLiteral("gunicorn")});
#else
#endif
	}
}

/***********************************************************************************/
void ServerManager::stopTHREDDS() {
	m_apacheProcess.setArguments({QStringLiteral("shutdown.sh")});

	m_isApacheRunning = m_apacheProcess.startDetached();
}
