#ifndef SYNCTHINGTRAY_NO_WEBVIEW
#include "./webviewdialog.h"
#include "./webpage.h"

#include "../application/settings.h"

#include <qtutilities/misc/dialogutils.h>

#include <QIcon>
#include <QCloseEvent>
#if defined(SYNCTHINGTRAY_USE_WEBENGINE)
# include <QWebEngineView>
#elif defined(SYNCTHINGTRAY_USE_WEBKIT)
# include <QWebView>
#endif

using namespace Dialogs;

namespace QtGui {

WebViewDialog::WebViewDialog(QWidget *parent) :
    QMainWindow(parent),
    m_view(new WEB_VIEW_PROVIDER(this))
{
    setWindowTitle(tr("Syncthing"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/hicolor/scalable/app/syncthingtray.svg")));
    setCentralWidget(m_view);

    m_view->setPage(new WebPage(this, m_view));
    connect(m_view, &WEB_VIEW_PROVIDER::titleChanged, this, &WebViewDialog::setWindowTitle);

    if(Settings::webViewGeometry().isEmpty()) {
        resize(1200, 800);
        centerWidget(this);
    } else {
        restoreGeometry(Settings::webViewGeometry());
    }
}

QtGui::WebViewDialog::~WebViewDialog()
{
    Settings::webViewGeometry() = saveGeometry();
}

void QtGui::WebViewDialog::applySettings(const Settings::ConnectionSettings &connectionSettings)
{
    m_settings = connectionSettings;
    m_view->setUrl(connectionSettings.syncthingUrl);
    m_view->setZoomFactor(Settings::webViewZoomFactor());
}

void QtGui::WebViewDialog::closeEvent(QCloseEvent *event)
{
    if(!Settings::webViewKeepRunning()) {
        deleteLater();
    }
    event->accept();
}

}

#endif // SYNCTHINGTRAY_NO_WEBVIEW
